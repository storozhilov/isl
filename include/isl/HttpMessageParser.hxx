#ifndef ISL__HTTP_MESSAGE_PARSER__HXX
#define ISL__HTTP_MESSAGE_PARSER__HXX

#include <isl/AbstractError.hxx>
#include <isl/Http.hxx>
#include <isl/Char.hxx>
#include <string>
#include <memory>

#ifndef ISL__HTTP_MESSAGE_PARSER_DEFAULT_MAX_HEADER_NAME_LENGTH
#define ISL__HTTP_MESSAGE_PARSER_DEFAULT_MAX_HEADER_NAME_LENGTH 256
#endif
#ifndef ISL__HTTP_MESSAGE_PARSER_DEFAULT_MAX_HEADER_VALUE_LENGTH
#define ISL__HTTP_MESSAGE_PARSER_DEFAULT_MAX_HEADER_VALUE_LENGTH 4096	// 4 Kb
#endif
#ifndef ISL__HTTP_MESSAGE_PARSER_DEFAULT_MAX_HEADERS_AMOUNT
#define ISL__HTTP_MESSAGE_PARSER_DEFAULT_MAX_HEADERS_AMOUNT 256
#endif

namespace isl
{

//! HTTP-message parser
/*!
  Parser does not apply strict rules on the first three tokens: first and second ones could consist of
  CHAR's which are not CTL/SP/HT's, third one is to be of CHAR's, which are not CTL's (see RFC-2616).
  You should use onFirstTokenParsed(), onSecondTokenParsed() and onThirdTokenParsed() event handlers to
  apply additional rules to theese elements.
*/
class HttpMessageParser
{
public:
	//! Class constants
	enum Constants {
		DefaultMaxHeaderNameLength = ISL__HTTP_MESSAGE_PARSER_DEFAULT_MAX_HEADER_NAME_LENGTH,
		DefaultMaxHeaderValueLength = ISL__HTTP_MESSAGE_PARSER_DEFAULT_MAX_HEADER_VALUE_LENGTH,
		DefaultMaxHeadersAmount = ISL__HTTP_MESSAGE_PARSER_DEFAULT_MAX_HEADERS_AMOUNT
	};
	//! Parser states
	enum State {
		ParsingMessage,					//!< Initial state
		ParsingFirstToken,				//!< Parsing first token
		ParsingFirstTokenSP,				//!< Parsing the delimeter b/w first and second token
		ParsingSecondToken,				//!< Parsing second token
		ParsingSecondTokenSP,				//!< Parsing the delimeter b/w second and third token
		ParsingThirdToken,				//!< Parsing third token
		ParsingFirstLineLF,				//!< First line LF has been found
		ParsingHeader,					//!< Parsing the beginning of the message header
		ParsingHeaderName,				//!< Parsing message header name
		ParsingHeaderValue,				//!< Parsing message header value
		ParsingHeaderValueLF,				//!< Message header line LF has been found
		ParsingHeaderValueLWS,				//!< Parsing message header multiline value LWS
		ParsingEndOfHeader,				//!< Parsing the end of the message header section
		ParsingIdentityBody,				//!< Parsing indentity-encoded message body
		ParsingChunkSize,				//!< Parsing the chunk size of the chunked-encoded message body
		ParsingChunkSizeLF,				//!< Chunk size line LF of of the chunked-encoded message body has been found
		ParsingChunkExtension,				//!< Parsing the chunk extension of the of the chunked-encoded message body
		ParsingChunk,					//!< Parsing the chunk of the chunked-encoded message body
		ParsingChunkCR,					//!< Chunk's CR has been found
		ParsingChunkLF,					//!< Chunk's LF has been found
		ParsingTrailerHeader,				//!< Parsing the beginning of the message trailer header
		ParsingTrailerHeaderName,			//!< Parsing message trailer header name
		ParsingTrailerHeaderValue,			//!< Parsing message trailer header value
		ParsingTrailerHeaderValueLF,			//!< Message trailer header line LF has been found
		ParsingTrailerHeaderValueLWS,			//!< Parsing message trailer header multiline value LWS
		ParsingFinalLF,					//!< Parsing the final LF of the message
		MessageCompleted				//!< Complete message has been parsed
	};
	//! HTTP-message parser error class
	class Error : public AbstractError
	{
	public:
		//! Constructs an HTTP-message parser error
		/*!
		  \param SOURCE_LOCATION_ARGS_DECLARATION Put SOURCE_LOCATION_ARGS macro here
		  \param ch Character, which caused an error
		  \param pos Position of the error in the HTTP-message
		  \param line Line of the error in the HTTP-message
		  \param col Column of the error in the HTTP-message
		  \param msg Error message
		*/
		Error(SOURCE_LOCATION_ARGS_DECLARATION, char ch, int pos, int line, int col, const std::string& msg);
		//! Returns a character, which caused an error
		inline char ch() const
		{
			return _ch;
		}
		//! Returns a position of the error in the HTTP-message
		inline int pos() const
		{
			return _pos;
		}
		//! Returns a line of the error in the HTTP-message
		inline int line() const
		{
			return _line;
		}
		//! Returns a column of the error in the HTTP-message
		inline int col() const
		{
			return _col;
		}
		//! Returns error message
		inline const std::string& msg() const
		{
			return _msg;
		}

		virtual AbstractError * clone() const
		{
			return new Error(*this);
		}
	private:
		virtual std::string composeMessage() const;

		const char _ch;
		const int _pos;
		const int _line;
		const int _col;
		const std::string _msg;
	};
	//! Constructs parser
	/*!
	  \param maxFirstTokenLength Maximum first token length
	  \param maxSecondTokenLength Maximum second token length
	  \param maxThirdTokenLength Maximum third token length
	  \param maxHeaderNameLength Maximum header name length
	  \param maxHeaderValueLength Maximum header value length
	  \param maxHeadersAmount Maximum headers amount
	*/
	HttpMessageParser(size_t maxFirstTokenLength, size_t maxSecondTokenLength, size_t maxThirdTokenLength,
			size_t maxHeaderNameLength = DefaultMaxHeaderNameLength,
			size_t maxHeaderValueLength = DefaultMaxHeaderValueLength,
			size_t maxHeadersAmount = DefaultMaxHeadersAmount);
	virtual ~HttpMessageParser();

	//! Inspects for bad HTTP-message
	inline bool isBad() const
	{
		return _errorAutoPtr.get();
	}
	//! Returns pointer to the error object or 0 if no error occured
	inline const AbstractError * error() const
	{
		return _errorAutoPtr.get();
	}
	//! Returns the current position of the HTTP-message parser (starts from 0)
	inline size_t pos() const
	{
		return _pos;
	}
	//! Returns the current line of the HTTP-message parser (starts from 1)
	inline size_t line() const
	{
		return _line;
	}
	//! Returns the current column of the HTTP-message parser (starts from 1)
	inline size_t col() const
	{
		return _col;
	}
	//! Returns a constant reference to the first token
	inline const std::string& firstToken() const
	{
		return _firstToken;
	}
	//! Returns a constant reference to the second token
	inline const std::string& secondToken() const
	{
		return _secondToken;
	}
	//! Returns a constant reference to the third token
	inline const std::string& thirdToken() const
	{
		return _thirdToken;
	}
	//! Returns a constant reference to the HTTP-message header
	inline const Http::Params& header() const
	{
		return _header;
	}
	//! Returns maximum header field name length
	inline size_t maxHeaderNameLength() const
	{
		return _maxHeaderNameLength;
	}
	//! Sets maximum header field name length
	/*!
	  \param newValue New maximum header field name length
	*/
	inline void setMaxHeaderNameLength(size_t newValue)
	{
		_maxHeaderNameLength = newValue;
	}
	//! Returns maximum header field value length
	inline size_t maxHeaderValueLength() const
	{
		return _maxHeaderValueLength;
	}
	//! Sets maximum header field value length
	/*!
	  \param newValue New maximum header field value length
	*/
	inline void setMaxHeaderValueLength(size_t newValue)
	{
		_maxHeaderValueLength = newValue;
	}
	//! Returns maximum headers amount
	inline size_t maxHeadersAmount() const
	{
		return _maxHeadersAmount;
	}
	//! Sets maximum headers amount
	/*!
	  \param newValue New maximum headers amount
	*/
	inline void setMaxHeadersAmount(size_t newValue)
	{
		_maxHeadersAmount = newValue;
	}
	//! Return the state of the parser
	inline State state() const
	{
		return _state;
	}
	//! Returns TRUE if the whole HTTP-message has been completely parsed
	inline bool isCompleted() const
	{
		return _state == MessageCompleted;
	}
	//! Parses next character
	/*!
	  \param ch Next character to parse
	  \return TRUE if the parsed character is the next HTTP-message body byte
	*/
	bool parse(char ch);
	//! Resets parser
	virtual void reset();
protected:
	//! Sets parser to the error state
	void setIsBad(char ch, const std::string& errMsg);
	//! On first token parsed event handler
	/*!
	  \param token Parsed token value
	*/
	virtual void onFirstTokenParsed(const std::string& token)
	{}
	//! On second token parsed event handler
	/*!
	  \param token Parsed token value
	*/
	virtual void onSecondTokenParsed(const std::string& token)
	{}
	//! On third token parsed event handler
	/*!
	  \param token Parsed token value
	*/
	virtual void onThirdTokenParsed(const std::string& token)
	{}
private:
	HttpMessageParser();

	void appendHeader(char ch);
	void parseHeader(char ch, bool isTrailer);
	void parseHeaderName(char ch, bool isTrailer);
	void parseHeaderValue(char ch, bool isTrailer);
	void parseHeaderValueLF(char ch, bool isTrailer);
	void parseHeaderValueLWS(char ch, bool isTrailer);

	State _state;
	std::auto_ptr<AbstractError> _errorAutoPtr;
	size_t _pos;
	size_t _line;
	size_t _col;
	std::string _firstToken;
	std::string _secondToken;
	std::string _thirdToken;
	std::string _headerFieldName;
	std::string _headerFieldValue;
	Http::Params _header;
	size_t _contentLength;
	size_t _identityBodyBytesParsed;
	std::string _chunkSizeStr;
	size_t _chunkSize;
	size_t _chunkBytesParsed;
	size_t _maxFirstTokenLength;
	size_t _maxSecondTokenLength;
	size_t _maxThirdTokenLength;
	size_t _maxHeaderNameLength;
	size_t _maxHeaderValueLength;
	size_t _maxHeadersAmount;
};

} // namespace isl

#endif

#ifndef ISL__HTTP_MESSAGE_STREAM_READER__HXX
#define ISL__HTTP_MESSAGE_STREAM_READER__HXX

#include <isl/AbstractIODevice.hxx>
#include <isl/Http.hxx>
#include <isl/Char.hxx>
#include <string>
#include <map>
#include <list>

namespace isl
{

//! Base class for HTTP-message stream readers
/*!
  TODO Use isl::AbstractError descendant for the error handling
*/
class HttpMessageStreamReader
{
public:
	//! Parser states
	enum ParserState {
		ParsingMessage,
		ParsingFirstToken,
		ParsingFirstTokenSP,
		ParsingSecondToken,
		ParsingSecondTokenSP,
		ParsingThirdToken,
		ParsingFirstLineLF,
		ParsingHeaderField,
		ParsingHeaderFieldName,
		ParsingHeaderFieldValue,
		ParsingHeaderFieldValueLF,
		ParsingHeaderFieldValueLWS,
		ParsingEndOfHeader,
		ParsingIdentityBody,
		ParsingChunkSize,
		ParsingChunkSizeLF,
		ParsingChunkExtension,
		ParsingChunk,
		ParsingChunkCR,
		ParsingChunkLF,
		ParsingTrailerHeaderField,
		ParsingTrailerHeaderFieldName,
		ParsingTrailerHeaderFieldValue,
		ParsingTrailerHeaderFieldValueLF,
		ParsingTrailerHeaderFieldValueLWS,
		ParsingFinalLF,
		MessageCompleted
	};

	//! Constructs reader
	/*!
	  \param device Reference to the I/O-device to fetch data from
	*/
	HttpMessageStreamReader(AbstractIODevice& device);
	virtual ~HttpMessageStreamReader();
	//! Inspects for bad HTTP-message
	inline bool isBad() const
	{
		return _isBad;
	}
	//! Returns HTTP-message parsing error
	inline std::string parsingError() const
	{
		return _parsingError;
	}
	//! Returns the current position of the HTTP-message parser
	inline size_t pos() const
	{
		return _pos;
	}
	//! Returns the current line of the HTTP-message parser
	inline size_t line() const
	{
		return _line;
	}
	//! Returns the current column of the HTTP-message parser
	inline size_t col() const
	{
		return _col;
	}
	//! Returns a constant referense to the HTTP-message header
	inline const Http::Params& header() const
	{
		return _header;
	}
	//! Returns maximum header field name length
	inline size_t maxHeaderFieldNameLength() const
	{
		return _maxHeaderFieldNameLength;
	}
	//! Sets maximum header field name length
	/*!
	  \param newValue New maximum header field name length
	*/
	inline void setMaxHeaderFieldNameLength(size_t newValue)
	{
		_maxHeaderFieldNameLength = newValue;
	}
	//! Returns maximum header field value length
	inline size_t maxHeaderFieldValueLength() const
	{
		return _maxHeaderFieldValueLength;
	}
	//! Sets maximum header field value length
	/*!
	  \param newValue New maximum header field value length
	*/
	inline void setMaxHeaderFieldValueLength(size_t newValue)
	{
		_maxHeaderFieldValueLength = newValue;
	}
	//! Reads data from the HTTP-message stream
	/*!
	  \param buffer Buffer for the data
	  \param bufferSize Data buffer size
	  \param timeout Timeout for reading operation
	  \return Number of bytes fetched
	*/
	size_t read(char * buffer, size_t bufferSize, const Timeout& timeout = Timeout(), bool * timeoutExpired = 0);
	//! Return the state of the parser
	inline ParserState parserState() const
	{
		return _parserState;
	}
	//! Returns TRUE if the HTTP-message has been completely fetched
	inline bool isCompleted() const
	{
		return _parserState == MessageCompleted;
	}

	//! Resets reader
	virtual void reset();
protected:
	inline void setIsBad(const std::string& parsingError)
	{
		_isBad = true;
		_parsingError = parsingError;
	}

	virtual bool isAllowedInFirstToken(char ch) const = 0;
	virtual void appendToFirstToken(char ch) = 0;
	virtual bool isAllowedInSecondToken(char ch) const = 0;
	virtual void appendToSecondToken(char ch) = 0;
	virtual bool isAllowedInThirdToken(char ch) const = 0;
	virtual void appendToThirdToken(char ch) = 0;
private:
	HttpMessageStreamReader();

	enum PrivateConstants {
		MaxHeaderFieldNameLength = 256,
		MaxHeaderFieldValueLength = 4096
	};
	
	//! Parsing next character method
	/*!
	  \param ch Next character to parse
	  \return true if next body byte has been fetched and saved to _bodyByte member
	*/
	bool parse(char ch);
	void appendHeader();

	void parseHeaderField(char ch, bool isTrailer);
	void parseHeaderFieldName(char ch, bool isTrailer);
	void parseHeaderFieldValue(char ch, bool isTrailer);
	void parseHeaderFieldValueLF(char ch, bool isTrailer);
	void parseHeaderFieldValueLWS(char ch, bool isTrailer);

	AbstractIODevice& _device;
	ParserState _parserState;
	bool _isBad;
	std::string _parsingError;
	size_t _pos;
	size_t _line;
	size_t _col;
	std::string _headerFieldName;
	std::string _headerFieldValue;
	Http::Params _header;
	size_t _contentLength;
	size_t _identityBodyBytesParsed;
	std::string _chunkSizeStr;
	size_t _chunkSize;
	size_t _chunkBytesParsed;
	char _bodyByte;
	size_t _maxHeaderFieldNameLength;
	size_t _maxHeaderFieldValueLength;

	friend class HttpRequestReader;
};

} // namespace isl

#endif

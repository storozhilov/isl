#ifndef ISL__HTTP_MESSAGE_STREAM_READER__HXX
#define ISL__HTTP_MESSAGE_STREAM_READER__HXX

#include <isl/AbstractIODevice.hxx>
#include <isl/Http.hxx>
#include <string>
#include <map>
#include <list>

namespace isl
{

//! Base class for HTTP-message stream writers
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

	HttpMessageStreamReader(AbstractIODevice& device);
	virtual ~HttpMessageStreamReader();

	inline bool isBad() const
	{
		return _isBad;
	}
	inline std::wstring parsingError() const
	{
		return _parsingError;
	}
	inline unsigned int pos() const
	{
		return _pos;
	}
	inline unsigned int line() const
	{
		return _line;
	}
	inline unsigned int col() const
	{
		return _col;
	}
	inline const Http::Header header() const
	{
		return _header;
	}
	inline const Http::Cookies cookies() const
	{
		return _cookies;
	}
	inline unsigned int maxHeaderFieldNameLength() const
	{
		return _maxHeaderFieldNameLength;
	}
	inline void setMaxHeaderFieldNameLength(unsigned int newValue)
	{
		_maxHeaderFieldNameLength = newValue;
	}
	inline unsigned int maxHeaderFieldValueLength() const
	{
		return _maxHeaderFieldValueLength;
	}
	inline void setMaxHeaderFieldValueLength(unsigned int newValue)
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
	unsigned int read(char * buffer, unsigned int bufferSize, const Timeout& timeout = Timeout(), bool * timeoutExpired = 0);
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
	//! Inspects HTTP-header for specified field name
	/*!
	  \param fieldName Name of the HTTP-header field.
	  \return TRUE if HTTP-header field exists.
	*/
	bool headerContains(const std::string &fieldName) const;
	//! Inspects HTTP-header for specified field name and value
	/*!
	  \param fieldName Name of the HTTP-header field.
	  \param fieldValue Value of the HTTP-header field.
	  \return TRUE if HTTP-header field exists.
	*/
	bool headerContains(const std::string &fieldName, const std::string &fieldValue) const;
	//! Returns HTTP-header value of the specified field
	/*!
	  \param fieldName Name of the HTTP-header field.
	  \return Value of the HTTP-header field or empty string if it does not exists
	*/
	std::string headerValue(const std::string &fieldName) const;
	//! Returns HTTP-header values list of the specified field
	/*!
	  \param fieldName Name of the HTTP-header field.
	  \return Values list of the HTTP-header field or empty list if it does not exists
	*/
	std::list<std::string> headerValues(const std::string &fieldName) const;

	//! Resets reader
	virtual void reset();
protected:
	inline void setIsBad(const std::wstring& parsingError)
	{
		_isBad = true;
		_parsingError = parsingError;
	}
	inline static bool isToken(unsigned char ch)
	{
		return (isChar(ch) && !isControl(ch) && !isSeparator(ch));
	}
	inline static bool isAlpha(unsigned char ch)
	{
		return isLowAlpha(ch) || isUpAlpha(ch);
	}
	inline static bool isDigit(unsigned char ch)
	{
		return ch >= '0' && ch <= '9';
	}
	inline static bool isAllowedInUri(unsigned char ch)
	{
		// See chaper A. of the RFC2936 (http://www.ietf.org/rfc/rfc2396.txt)
		return isAlpha(ch) || isDigit(ch) || ch == '#' || ch == ':' || ch == '?' || ch == ';' || ch == '@' ||
			ch == '&' || ch == '=' || ch == '+' || ch == '$' || ch == ',' || ch == '-' || ch == '.' ||
			ch == '/' || ch == '_' || ch == '!' || ch == '~' || ch == '*' || ch == '\'' || ch == '(' ||
			ch == ')' || ch == '%';
	}
	inline static bool isChar(unsigned char ch)
	{
		return (ch <= 0x7F);
	}
	inline static bool isLowAlpha(unsigned char ch)
	{
		return ch >= 'a' && ch <= 'z';
	}
	inline static bool isUpAlpha(unsigned char ch)
	{
		return ch >= 'A' && ch <= 'Z';
	}
	inline static bool isHexDigit(unsigned char ch)
	{
		return isDigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
	}
	inline static bool isControl(unsigned char ch)
	{
		return (ch <= 0x1F) || (ch == 0x7F);
	}
	inline static bool isCarriageReturn(unsigned char ch)
	{
		return (ch == 0x0D);
	}
	inline static bool isLineFeed(unsigned char ch)
	{
		return (ch == 0x0A);
	}
	inline static bool isSpace(unsigned char ch)
	{
		return (ch == 0x20);
	}
	inline static bool isTab(unsigned char ch)
	{
		return (ch == 0x09);
	}
	inline static bool isSpaceOrTab(unsigned char ch)
	{
		return (isSpace(ch) || isTab(ch));
	}
	inline static bool isSeparator(unsigned char ch)
	{
		return	(ch == '(') || (ch == ')') || (ch == '<') || (ch == '>') || (ch == '@') ||
			(ch == ',') || (ch == ';') || (ch == ':') || (ch == '\\') || (ch == '"') ||
			(ch == '/') || (ch == '[') || (ch == ']') || (ch == '?') || (ch == '=') ||
			(ch == '{') || (ch == '}') || isSpaceOrTab(ch);
	}
	inline static bool isAllowedInVersion(unsigned char ch)
	{
		return isDigit(ch) || ch == 'H' || ch == 'T' || ch == 'P' || ch == '/' || ch == '.';
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

	inline static bool isAllowedInHeader(unsigned char ch)
	{
		return isChar(ch);
	}

	AbstractIODevice& _device;
	ParserState _parserState;
	bool _isBad;
	std::wstring _parsingError;
	unsigned int _pos;
	unsigned int _line;
	unsigned int _col;
	std::string _headerFieldName;
	std::string _headerFieldValue;
	Http::Header _header;
	Http::Cookies _cookies;
	unsigned int _contentLength;
	unsigned int _identityBodyBytesParsed;
	std::string _chunkSizeStr;
	unsigned int _chunkSize;
	unsigned int _chunkBytesParsed;
	char _bodyByte;
	unsigned int _maxHeaderFieldNameLength;
	unsigned int _maxHeaderFieldValueLength;
};

} // namespace isl

#endif

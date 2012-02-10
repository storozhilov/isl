#ifndef ISL__HTTP_REQUEST_STREAM_READER__HXX
#define ISL__HTTP_REQUEST_STREAM_READER__HXX

#include <isl/HttpMessageStreamReader.hxx>
//#include <isl/AbstractIODevice.hxx>
//#include <isl/AbstractError.hxx>
//#include <string>
//#include <map>
//#include <list>

namespace isl
{

////! HTTP-request stream reader
//class HttpRequestStreamReader
//{
//public:
//	// TODO Migrate to it
//	class Error : public AbstractInfoError
//	{
//	public:
//		Error(SOURCE_LOCATION_ARGS_DECLARATION, const HttpRequestStreamReader& reader, const std::wstring& info) :
//			AbstractInfoError(SOURCE_LOCATION_ARGS_PASSTHRU, info),
//			_pos(reader.pos()),
//			_line(reader.line()),
//			_col(reader.col())
//		{}
//
//		virtual AbstractError * clone() const
//		{
//			return new Error(*this);
//		}
//	private:
//		Error();
//		
//		virtual std::wstring composeMessage() const
//		{
//			return info();
//			/*std::wstring result;
//			switch (_type) {
//				case TimeoutExpired:
//					result = L"Timeout expired on I/O-device";
//					break;
//				case ConnectionAborted:
//					result = L"Connection aborted on I/O-device";
//					break;
//				case DeviceIsNotOpen:
//					result = L"I/O-device is not open";
//					break;
//				default:
//					result = L"Unknown I/O-error";
//			}
//			appendInfo(result);
//			return result;*/
//		}
//
//		unsigned int _pos;
//		unsigned int _line;
//		unsigned int _col;
//	};
//
//	//! Parser states
//	enum ParserState {
//		ParsingRequest,
//		ParsingMethod,
//		ParsingMethodUriDelimeter,
//		ParsingUri,
//		ParsingUriVersionDelimeter,
//		ParsingVersion,
//		ParsingVersionLF,
//		ParsingHeaderField,
//		ParsingHeaderFieldName,
//		ParsingHeaderFieldValue,
//		ParsingHeaderFieldValueLF,
//		ParsingHeaderFieldValueLWS,
//		ParsingEndOfHeader,
//		ParsingIdentityBody,
//		ParsingChunkSize,
//		ParsingChunkSizeLF,
//		ParsingChunkExtension,
//		ParsingChunk,
//		ParsingChunkCR,
//		ParsingChunkLF,
//		ParsingTrailerHeaderField,
//		ParsingTrailerHeaderFieldName,
//		ParsingTrailerHeaderFieldValue,
//		ParsingTrailerHeaderFieldValueLF,
//		ParsingTrailerHeaderFieldValueLWS,
//		ParsingFinalLF,
//		RequestCompleted,
//		// Error states
//		BadRequest,
//		RequestMethodTooLong,				// TODO Remove it -> error info in exception
//		RequestUriTooLong,				// TODO Remove it -> error info in exception
//		RequestVersionTooLong,				// TODO Remove it -> error info in exception
//		RequestHeaderFieldNameTooLong,			// TODO Remove it -> error info in exception
//		RequestHeaderFieldValueTooLong,			// TODO Remove it -> error info in exception
//		RequestEntityTooLong,				// TODO Remove it -> error info in exception
//		InvalidState					// TODO Remove it -> error info in exception?
//	};
//
//	typedef std::multimap<std::string, std::string> Header;
//	typedef std::map<std::string, std::string> Cookies;
//
//	HttpRequestStreamReader(AbstractIODevice& device);
//	//virtual ~HttpRequestStreamReader();
//
//	inline unsigned int pos() const
//	{
//		return _pos;
//	}
//	inline unsigned int line() const
//	{
//		return _line;
//	}
//	inline unsigned int col() const
//	{
//		return _col;
//	}
//	inline const std::string& method() const
//	{
//		return _method;
//	}
//	inline const std::string& uri() const
//	{
//		return _uri;
//	}
//	inline const std::string& version() const
//	{
//		return _version;
//	}
//	inline const Header& header() const
//	{
//		return _header;
//	}
//	inline const Cookies& cookies() const
//	{
//		return _cookies;
//	}
//	inline unsigned int maxMethodLength() const
//	{
//		return _maxMethodLength;
//	}
//	inline void setMaxMethodLength(unsigned int newValue)
//	{
//		_maxMethodLength = newValue;
//	}
//	inline unsigned int maxUriLength() const
//	{
//		return _maxUriLength;
//	}
//	inline void setMaxUriLength(unsigned int newValue)
//	{
//		_maxUriLength = newValue;
//	}
//	inline unsigned int maxVersionLength() const
//	{
//		return _maxVersionLength;
//	}
//	inline void setMaxVersionLength(unsigned int newValue)
//	{
//		_maxVersionLength = newValue;
//	}
//	inline unsigned int maxHeaderFieldNameLength() const
//	{
//		return _maxHeaderFieldNameLength;
//	}
//	inline void setMaxHeaderFieldNameLength(unsigned int newValue)
//	{
//		_maxHeaderFieldNameLength = newValue;
//	}
//	inline unsigned int maxHeaderFieldValueLength() const
//	{
//		return _maxHeaderFieldValueLength;
//	}
//	inline void setMaxHeaderFieldValueLength(unsigned int newValue)
//	{
//		_maxHeaderFieldValueLength = newValue;
//	}
//	//! Resets reader
//	void reset();
//	//! Reads data from the HTTP-request stream
//	/*!
//	  \param buffer Buffer for the data
//	  \param bufferSize Data buffer size
//	  \param timeout Timeout for reading operation
//	  \return Number of bytes fetched
//	*/
//	unsigned int read(char * buffer, unsigned int bufferSize, const Timeout& timeout = Timeout());
//	//! Return the state of the parser
//	inline ParserState parserState() const
//	{
//		return _parserState;
//	}
//	//! Returns TRUE if the HTTP-request has been completely fetched
//	inline bool requestCompleted() const
//	{
//		return _parserState == RequestCompleted;
//	}
//	//! Returns TRUE if invalid HTTP-request has been detected
//	bool invalidRequest() const;
//	//! Inspects HTTP-header for specified field name
//	/*!
//	  \param fieldName Name of the HTTP-header field.
//	  \return TRUE if HTTP-header field exists.
//	*/
//	bool headerContains(const std::string &fieldName) const;
//	//! Inspects HTTP-header for specified field name and value
//	/*!
//	  \param fieldName Name of the HTTP-header field.
//	  \param fieldValue Value of the HTTP-header field.
//	  \return TRUE if HTTP-header field exists.
//	*/
//	bool headerContains(const std::string &fieldName, const std::string &fieldValue) const;
//	//! Returns HTTP-header value of the specified field
//	/*!
//	  \param fieldName Name of the HTTP-header field.
//	  \return Value of the HTTP-header field or empty string if it does not exists
//	*/
//	std::string headerValue(const std::string &fieldName) const;
//	//! Returns HTTP-header values list of the specified field
//	/*!
//	  \param fieldName Name of the HTTP-header field.
//	  \return Values list of the HTTP-header field or empty list if it does not exists
//	*/
//	std::list<std::string> headerValues(const std::string &fieldName) const;
//private:
//	HttpRequestStreamReader();
//	HttpRequestStreamReader(const HttpRequestStreamReader&);							// No copy
//
//	HttpRequestStreamReader& operator=(const HttpRequestStreamReader&);						// No copy
//
//	enum PrivateConstants {
//		MaxMethodLength = 20,
//		MaxUriLength = 4096,
//		MaxVersionLength = 20,
//		MaxHeaderFieldNameLength = 256,
//		MaxHeaderFieldValueLength = 4096
//	};
//	
//	//! Parsing next character method
//	/*!
//	  \param ch Nexta character to parse
//	  \return true if next body byte has been fetched and saved to _bodyByte member
//	*/
//	bool parse(char ch);
//	void appendHeader();
//
//	void parseHeaderField(char ch, bool isTrailer);
//	void parseHeaderFieldName(char ch, bool isTrailer);
//	void parseHeaderFieldValue(char ch, bool isTrailer);
//	void parseHeaderFieldValueLF(char ch, bool isTrailer);
//	void parseHeaderFieldValueLWS(char ch, bool isTrailer);
//
//	inline static bool isChar(unsigned char ch)
//	{
//		return (ch <= 0x7F);
//	}
//	inline static bool isLowAlpha(unsigned char ch)
//	{
//		return ch >= 'a' && ch <= 'z';
//	}
//	inline static bool isUpAlpha(unsigned char ch)
//	{
//		return ch >= 'A' && ch <= 'Z';
//	}
//	inline static bool isAlpha(unsigned char ch)
//	{
//		return isLowAlpha(ch) || isUpAlpha(ch);
//	}
//	inline static bool isDigit(unsigned char ch)
//	{
//		return ch >= '0' && ch <= '9';
//	}
//	inline static bool isHexDigit(unsigned char ch)
//	{
//		return isDigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
//	}
//	inline static bool isControl(unsigned char ch)
//	{
//		return (ch <= 0x1F) || (ch == 0x7F);
//	}
//	inline static bool isCarriageReturn(unsigned char ch)
//	{
//		return (ch == 0x0D);
//	}
//	inline static bool isLineFeed(unsigned char ch)
//	{
//		return (ch == 0x0A);
//	}
//	inline static bool isSpace(unsigned char ch)
//	{
//		return (ch == 0x20);
//	}
//	inline static bool isTab(unsigned char ch)
//	{
//		return (ch == 0x09);
//	}
//	inline static bool isSpaceOrTab(unsigned char ch)
//	{
//		return (isSpace(ch) || isTab(ch));
//	}
//	inline static bool isSeparator(unsigned char ch)
//	{
//		return	(ch == '(') || (ch == ')') || (ch == '<') || (ch == '>') || (ch == '@') ||
//			(ch == ',') || (ch == ';') || (ch == ':') || (ch == '\\') || (ch == '"') ||
//			(ch == '/') || (ch == '[') || (ch == ']') || (ch == '?') || (ch == '=') ||
//			(ch == '{') || (ch == '}') || isSpaceOrTab(ch);
//	}
//	inline static bool isToken(unsigned char ch)
//	{
//		return (isChar(ch) && !isControl(ch) && !isSeparator(ch));
//	}
//	inline static bool isAllowedInUri(unsigned char ch)
//	{
//		// See chaper A. of the RFC2936 (http://www.ietf.org/rfc/rfc2396.txt)
//		return isAlpha(ch) || isDigit(ch) || ch == '#' || ch == ':' || ch == '?' || ch == ';' || ch == '@' ||
//			ch == '&' || ch == '=' || ch == '+' || ch == '$' || ch == ',' || ch == '-' || ch == '.' ||
//			ch == '/' || ch == '_' || ch == '!' || ch == '~' || ch == '*' || ch == '\'' || ch == '(' ||
//			ch == ')' || ch == '%';
//	}
//	inline static bool isAllowedInVersion(unsigned char ch)
//	{
//		return isDigit(ch) || ch == 'H' || ch == 'T' || ch == 'P' || ch == '/' || ch == '.';
//	}
//	inline static bool isAllowedInHeader(unsigned char ch)
//	{
//		return isChar(ch);
//	}
//
//	AbstractIODevice& _device;
//	ParserState _parserState;
//	unsigned int _pos;
//	unsigned int _line;
//	unsigned int _col;
//	std::string _method;
//	std::string _rawUri;
//	std::string _uri;
//	std::string _version;
//	std::string _headerFieldName;
//	std::string _headerFieldValue;
//	Header _header;
//	Cookies _cookies;
//	unsigned int _contentLength;
//	unsigned int _identityBodyBytesParsed;
//	std::string _chunkSizeStr;
//	unsigned int _chunkSize;
//	unsigned int _chunkBytesParsed;
//	char _bodyByte;
//	unsigned int _maxMethodLength;
//	unsigned int _maxUriLength;
//	unsigned int _maxVersionLength;
//	unsigned int _maxHeaderFieldNameLength;
//	unsigned int _maxHeaderFieldValueLength;
//};

class HttpRequestStreamReader : public HttpMessageStreamReader
{
public:
	HttpRequestStreamReader(AbstractIODevice& device) :
		HttpMessageStreamReader(device),
		_method(),
		_rawUri(),
		_version(),
		_maxMethodLength(MaxMethodLength),
		_maxUriLength(MaxUriLength),
		_maxVersionLength(MaxVersionLength)
	{}

	inline std::string method() const
	{
		return _method;
	}
	inline std::string uri() const
	{
		return String::urlDecode(_rawUri);
	}
	inline std::string version() const
	{
		return _version;
	}
	inline unsigned int maxMethodLength() const
	{
		return _maxMethodLength;
	}
	inline void setMaxMethodLength(unsigned int newValue)
	{
		_maxMethodLength = newValue;
	}
	inline unsigned int maxUriLength() const
	{
		return _maxUriLength;
	}
	inline void setMaxUriLength(unsigned int newValue)
	{
		_maxUriLength = newValue;
	}
	inline unsigned int maxVersionLength() const
	{
		return _maxVersionLength;
	}
	inline void setMaxVersionLength(unsigned int newValue)
	{
		_maxVersionLength = newValue;
	}

	virtual void reset()
	{
		HttpMessageStreamReader::reset();
		_method.clear();
		_rawUri.clear();
		_version.clear();
	}
private:
	HttpRequestStreamReader();

	enum PrivateConstants {
		MaxMethodLength = 20,
		MaxUriLength = 4096,
		MaxVersionLength = 20
	};
	
	virtual bool isAllowedInFirstToken(char ch) const
	{
		return isToken(ch);
	}
	virtual void appendToFirstToken(char ch)
	{
		if (_method.length() >= _maxMethodLength) {
			setIsBad(L"Request method is too long");
			return;
		}
		_method.append(1, ch);
	}
	virtual bool isAllowedInSecondToken(char ch) const
	{
		return isAllowedInUri(ch);
	}
	virtual void appendToSecondToken(char ch)
	{
		if (_rawUri.length() >= _maxUriLength) {
			setIsBad(L"Request URI is too long");
			return;
		}
		_rawUri.append(1, ch);
	}
	virtual bool isAllowedInThirdToken(char ch) const
	{
		return isAllowedInVersion(ch);
	}
	virtual void appendToThirdToken(char ch)
	{
		if (_version.length() >= _maxVersionLength) {
			setIsBad(L"Request version is too long");
			return;
		}
		_version.append(1, ch);
	}

	std::string _method;
	std::string _rawUri;
	std::string _version;
	unsigned int _maxMethodLength;
	unsigned int _maxUriLength;
	unsigned int _maxVersionLength;
};

} // namespace isl

#endif

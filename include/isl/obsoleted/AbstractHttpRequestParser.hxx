#ifndef ISL__ABSTRACT_HTTP_REQUEST_PARSER__HXX
#define ISL__ABSTRACT_HTTP_REQUEST_PARSER__HXX

#include <string>
#include <map>

namespace isl
{

/*------------------------------------------------------------------------------
  Simple state-based HTTP-request parser
------------------------------------------------------------------------------*/

class AbstractHttpRequestParser
{
public:
	// TODO Use isl::Enum as State (???)
	enum State {
		ParsingRequest,
		ParsingMethod,
		ParsingMethodUriDelimeter,
		ParsingUri,
		//ParsingAsteriskUri,
		//ParsingUriScheme,				// TODO
		//ParsingUriPath,
		ParsingUriVersionDelimeter,
		ParsingVersion,
		ParsingVersionCR,
		ParsingVersionLF,
		ParsingHeaderField,
		ParsingHeaderFieldName,
		ParsingHeaderFieldValue,
		//ParsingHeaderFieldValueCRLF,
		ParsingHeaderFieldValueLF,
		ParsingHeaderFieldValueLWS,
		ParsingEndOfHeader,
		ParsingBody,
		ParsingCompleted,
		// Error states
		BadRequest,
		RequestMethodTooLong,
		RequestUriTooLong,
		RequestVersionTooLong,
		RequestHeaderFieldNameTooLong,
		RequestHeaderFieldValueTooLong,
		RequestEntityTooLong,
		MethodNotImplemented,
		HTTPVersionNotImplemented,
		InvalidRequestURI,
		InvalidState
		// ... TODO
	};

	typedef std::multimap<std::string, std::string> Header;

	AbstractHttpRequestParser();
	virtual ~AbstractHttpRequestParser();

	void reset();
	int parse(const char * data, unsigned int size);
	void parse(char ch);
	bool needMoreData() const;
	State state() const;
	bool isBadRequest() const;
	bool isCompleteRequest() const;
	bool bodyExpected() const;
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
	inline const std::string& method() const
	{
		return _method;
	}
	inline const std::string& uri() const
	{
		return _uri;
	}
	inline const std::string& version() const
	{
		return _version;
	}
	inline const Header& header() const
	{
		return _header;
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
protected:
	virtual void methodParsed(const std::string& method) = 0;
	virtual void uriParsed(const std::string& uri) = 0;
	virtual void versionParsed(const std::string& version) = 0;
	virtual void headerFieldParsed(const std::string& fieldName, const std::string& fieldValue) = 0;
	virtual void bodyChunkParsed(const std::string& bodyChunk) = 0;

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
	inline static bool isAlpha(unsigned char ch)
	{
		return isLowAlpha(ch) || isUpAlpha(ch);
	}
	inline static bool isDigit(unsigned char ch)
	{
		return ch >= '0' && ch <= '9';
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
	inline static bool isToken(unsigned char ch)
	{
		return (isChar(ch) && !isControl(ch) && !isSeparator(ch));
	}
	inline static bool isAllowedInUri(unsigned char ch)
	{
		// See chaper A. of the RFC2936 (http://www.ietf.org/rfc/rfc2396.txt)
		return isAlpha(ch) || isDigit(ch) || ch == '#' || ch == ':' || ch == '?' || ch == ';' || ch == '@' ||
			ch == '&' || ch == '=' || ch == '+' || ch == '$' || ch == ',' || ch == '-' || ch == '.' ||
			ch == '/' || ch == '_' || ch == '!' || ch == '~' || ch == '*' || ch == '\'' || ch == '(' ||
			ch == ')' || ch == '%';
	}
	inline static bool isAllowedInVersion(unsigned char ch)
	{
		return isDigit(ch) || ch == 'H' || ch == 'T' || ch == 'P' || ch == '/' || ch == '.';
	}
	inline static bool isAllowedInHeader(unsigned char ch)
	{
		return isChar(ch);
	}
private:
	void methodParsedPrivate();
	void uriParsedPrivate();
	void versionParsedPrivate();
	void headerFieldParsedPrivate();
	void bodyChunkParsedPrivate();

	State _state;
	std::string _method;
	std::string _uri;
	std::string _version;
	std::string _headerFieldName;
	std::string _headerFieldValue;
	Header _header;
	std::string _bodyChunk;
	unsigned int _pos;
	unsigned int _line;
	unsigned int _col;
	unsigned int _maxMethodLength;
	unsigned int _maxUriLength;
	unsigned int _maxVersionLength;
	unsigned int _maxHeaderFieldNameLength;
	unsigned int _maxHeaderFieldValueLength;
};

} // namespace isl

#endif


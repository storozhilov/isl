#ifndef ISL__HTTP_REQUEST_COOKIE_PARSER__HXX
#define ISL__HTTP_REQUEST_COOKIE_PARSER__HXX

#include <isl/Http.hxx>
#include <isl/Error.hxx>
#include <string>
#include <sstream>

namespace isl
{

//! HTTP-request cookie parser
class HttpRequestCookieParser
{
public:
	//! Parser states
	enum ParserState {
		ParsingCookie,
		ParsingAttribute,
		ParsingAttributeSP,
		ParsingEquals,
		ParsingQuotedValue,
		ParsingQuotedValueBackslash,
		ParsingValue,
		ParsingValueSP
	};
	//! Composer states
	enum ComposerState {
		AwaitingVersion,
		AwaitingValue,
		AwaitingPath,
		AwaitingDomain,
		AwaitingPort
	};

	HttpRequestCookieParser() :
		_parserState(ParsingCookie),
		_composerState(AwaitingVersion),
		_pos(0),
		_curChar(0),
		_isBad(false),
		_error(),
		_cookieName(),
		_cookieValue(),
		_cookieVersion(),
		_cookiePath(),
		_cookieDomain(),
		_cookiePort(),
		_currentAttrName(),
		_currentAttrValue()
	{}
	//! Returns parser state
	inline ParserState parserState() const
	{
		return _parserState;
	}
	//! Returns composer state
	inline ComposerState composerState() const
	{
		return _composerState;
	}
	//! Returns current parsing position
	inline size_t pos() const
	{
		return _pos;
	}
	//! Returns current parsing character
	inline char curChar() const
	{
		return _curChar;
	}
	//! Returns TRUE if parsing error has been detected
	inline bool isBad() const
	{
		return _isBad;
	}
	//! Returns parser error
	inline std::string error() const
	{
		return _error;
	}
	//! Resets parser to it's initial state
	void reset();
	//! Parses cookie header value for qookies
	Http::RequestCookies parse(const std::string& headerValue);
private:
	inline void setIsBad(const std::string& errorMsg)
	{
		_isBad = true;
		_error = errorMsg;
	}
	void appendAttribute(Http::RequestCookies& parsedCookies, bool endOfCookieDetected);
	void appendCookie(Http::RequestCookies& parsedCookies);

	ParserState _parserState;
	ComposerState _composerState;
	size_t _pos;
	char _curChar;
	bool _isBad;
	std::string _error;
	std::string _cookieName;
	std::string _cookieValue;
	std::string _cookieVersion;
	std::string _cookiePath;
	std::string _cookieDomain;
	std::string _cookiePort;
	std::string _currentAttrName;
	std::string _currentAttrValue;
};

} // namespace isl

#endif

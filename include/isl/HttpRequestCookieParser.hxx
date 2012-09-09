#ifndef ISL__HTTP_REQUEST_COOKIE_PARSER__HXX
#define ISL__HTTP_REQUEST_COOKIE_PARSER__HXX

#include <isl/Http.hxx>
#include <isl/Error.hxx>
#include <string>
#include <sstream>
#include <memory>

namespace isl
{

//! HTTP-request cookie parser
/*!
  Parses HTTP-header value for cookies according to <a href="http://www.ietf.org/rfc/rfc2965.txt">RFC 2965</a>.
*/
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
		_errorAutoPtr(),
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
		//return _isBad;
		return _errorAutoPtr.get();
	}
	//! Returns a pointer to parsing error or 0 if no error occured
	inline AbstractError * error() const
	{
		return _errorAutoPtr.get();
	}
	//! Resets parser to it's initial state
	void reset();
	//! Parses cookie header value for cookies
	/*!
	  \param headerValue HTTP-header value
	  \param cookies Reference to cookies container to store result in
	*/
	void parse(const std::string& headerValue, Http::RequestCookies& cookies);
private:
	inline void setIsBad(const AbstractError& err)
	{
		_errorAutoPtr.reset(err.clone());
	}
	void appendAttribute(Http::RequestCookies& parsedCookies, bool endOfCookieDetected);
	void appendCookie(Http::RequestCookies& parsedCookies);

	ParserState _parserState;
	ComposerState _composerState;
	size_t _pos;
	char _curChar;
	std::auto_ptr<AbstractError> _errorAutoPtr;
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

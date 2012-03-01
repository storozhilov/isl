#ifndef ISL__HTTP__HXX
#define ISL__HTTP__HXX

#include <isl/Log.hxx>		// TODO Remove it
#include <isl/Char.hxx>
#include <map>
#include <list>

//! TODO Remove all rocket science from the HTTP subsystem!!!

namespace isl
{

//! Basic HTTP-routines class
class Http
{
public:
	typedef std::multimap<std::string, std::string> Header;
	//! For get and post
	typedef std::map<std::string, std::string> Params;

	struct RequestCookie {
		std::string name;
		std::string value;
		std::string version;
		std::string path;
		std::string domain;
		std::string port;

	};
	typedef std::multimap<std::string, RequestCookie> RequestCookies;
	/*struct ResponseCookie {
	};*/

	static Log errorLog;			// TODO To be removed
	static Log warningLog;			// TODO To be removed
	static Log debugLog;			// TODO To be removed
	static Log accessLog;			// TODO To be removed

	static void parseUri(const std::string& uriStr, std::string& path, std::string& query);
	static std::string composeUri(const std::string& path, const std::string& query = std::string());
	static void parseParams(const std::string& paramsStr, Params& params);
	static std::string composeParams(const Params& params);

	inline static bool isText(unsigned char ch)
	{
		return !isControl(ch) || Char::isCarriageReturn(ch) || Char::isLineFeed(ch) || Char::isTab(ch);
	}
	inline static bool isToken(unsigned char ch)
	{
		return (isChar(ch) && !isControl(ch) && !isSeparator(ch));
	}
	inline static bool isAlpha(unsigned char ch)
	{
		return isLowAlpha(ch) || isUpAlpha(ch);
	}
	inline static bool isAllowedInUri(unsigned char ch)
	{
		// See chaper A. of the RFC2936 (http://www.ietf.org/rfc/rfc2396.txt)
		return isAlpha(ch) || Char::isDigit(ch) || ch == '#' || ch == ':' || ch == '?' || ch == ';' || ch == '@' ||
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
	inline static bool isControl(unsigned char ch)
	{
		return (ch <= 0x1F) || (ch == 0x7F);
	}
	inline static bool isSeparator(unsigned char ch)
	{
		return	(ch == '(') || (ch == ')') || (ch == '<') || (ch == '>') || (ch == '@') ||
			(ch == ',') || (ch == ';') || (ch == ':') || (ch == '\\') || (ch == '"') ||
			(ch == '/') || (ch == '[') || (ch == ']') || (ch == '?') || (ch == '=') ||
			(ch == '{') || (ch == '}') || Char::isSpaceOrTab(ch);
	}
	inline static bool isAllowedInVersion(unsigned char ch)
	{
		return Char::isDigit(ch) || ch == 'H' || ch == 'T' || ch == 'P' || ch == '/' || ch == '.';
	}
};

} // namespace isl

#endif

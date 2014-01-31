#ifndef ISL__HTTP__HXX
#define ISL__HTTP__HXX

#include <isl/Char.hxx>
#include <string>
#include <map>
#include <list>
#include <functional>
#include <cstring>

namespace isl
{

//! Basic HTTP-routines class
class Http
{
public:
	//! HTTP datetime narrow character format string
	static const char * DateTimeFormat;
	//! HTTP datetime wide character format string
	static const wchar_t * DateTimeWFormat;

	struct CaseInsensitiveCompare : public std::binary_function<std::string, std::string, bool>
	{
		bool operator()(const std::string &lhs, const std::string &rhs) const {
			return strcasecmp(lhs.c_str(), rhs.c_str()) < 0 ;
		}
	};
	//! Container for headers (TODO: Use it for headers)
	typedef std::multimap<std::string, std::string, CaseInsensitiveCompare> Headers;
	//! Container for header, get and post data (TODO: Use it for GET and POST data only)
	typedef std::multimap<std::string, std::string> Params;
	//! HTTP-request cookie
	struct RequestCookie {
		std::string name;
		std::string value;
		std::string version;
		std::string path;
		std::string domain;
		std::string port;

	};
	//! HTTP-request cookies map
	typedef std::multimap<std::string, RequestCookie> RequestCookies;
	//! HTTP-reaponse cookie
	struct ResponseCookie {
		std::string name;
		std::string value;
		std::string comment;
		std::string commentUrl;
		bool discard;
		std::string domain;
		std::string maxAge;
		std::string path;
		std::string port;
		bool secure;
		std::string version;
	};
	//! HTTP-reaponse cookies map
	typedef std::multimap<std::string, ResponseCookie> ResponseCookies;
	
	//! Parses URI
	/*!
	  \param uriStr String to parse
	  \param path Reference to a string where percent-decoded URI's path should be stored in
	  \param path Reference to a string where raw URI's query should be stored in
	*/
	static void parseUri(const std::string& uriStr, std::string& path, std::string& query);
	//! Composes URI
	/*!
	  \param path Path part of the URI
	  \param query Query part of the URI
	  \return Composed URI
	*/
	static std::string composeUri(const std::string& path, const std::string& query = std::string());
	//! Extracts parameters from the percent-encoded string
	/*!
	  \param paramsStr String to parse
	  \param params Reference to container where params should be stored in
	*/
	static void parseParams(const std::string& paramsStr, Params& params);
	//! Encodes params
	/*!
	  \param params Parameters to encode
	  \return Encoded parameters string
	*/
	//! Inspects headers for header
	/*!
	 * \param headers Headers to inspect
	 * \param headerName Header to inspect for existence
	 * \return TRUE if header exists in headers
	 */
	inline static bool hasHeader(const Headers& headers, const std::string& header)
	{
		std::pair<Headers::const_iterator, Headers::const_iterator> range = headers.equal_range(header);
		return range.first != range.second;
	}
	//! Returns first header value
	inline static std::string headerValue(const Headers& headers, const std::string& header)
	{
		std::pair<Headers::const_iterator, Headers::const_iterator> range = headers.equal_range(header);
		return range.first == range.second ? std::string() : range.first->second;
	}
	//! Encodes params
	/*!
	  \param params Parameters to encode
	  \return Encoded parameters string
	*/
	static std::string composeParams(const Params& params);
	inline static bool hasParam(const Params& params, const std::string& paramName)
	{
		std::pair<Params::const_iterator, Params::const_iterator> range = params.equal_range(paramName);
		return range.first != range.second;
	}
	static bool hasParam(const Params& params, const std::string& paramName, const std::string& paramValue)
	{
		std::pair<Params::const_iterator, Params::const_iterator> range = params.equal_range(paramName);
		for (Params::const_iterator i = range.first; i != range.second; ++i) {
			if (i->second == paramValue) {
				return true;
			}
		}
		return false;
	}
	inline static std::string paramValue(const Params& params, const std::string& paramName)
	{
		std::pair<Params::const_iterator, Params::const_iterator> range = params.equal_range(paramName);
		return range.first == range.second ? std::string() : range.first->second;
	}
	static std::list<std::string> paramValues(const Params& params, const std::string &paramName)
	{
		std::list<std::string> result;
		std::pair<Params::const_iterator, Params::const_iterator> range = params.equal_range(paramName);
		for (Params::const_iterator i = range.first; i != range.second; ++i) {
			result.push_back(i->second);
		}
		return result;
	}
	//! Extracts HTTP-request cookies from the HTTP header
	/*!
	  \param header Header to extract cookies from
	  \param cookies Container to store the result
	*/
	static void grabCookies(const Params& header, RequestCookies& cookies);
	//! Extracts HTTP-response cookies from the HTTP header
	/*!
	  \param header Header to extract cookies from
	  \param cookies Container to store the result
	*/
	static void grabCookies(const Params& header, ResponseCookies& cookies);

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

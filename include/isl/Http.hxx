#ifndef ISL__HTTP__HXX
#define ISL__HTTP__HXX

#include <isl/Log.hxx>		// TODO Remove it
#include <map>

//! TODO Remove all rocket science from the HTTP subsystem!!!

namespace isl
{

//typedef std::multimap<std::string, std::string> HttpHeader;
//typedef std::map<std::string, std::string> HttpCookies;

// TODO Remove it
class Http
{
public:
	typedef std::multimap<std::string, std::string> Header;
	typedef std::map<std::string, std::string> Cookies;
	typedef std::map<std::string, std::string> Get;
	typedef std::map<std::string, std::string> Post;

	struct Uri {
		std::string scheme;
		std::string host;
		unsigned int port;
		std::string user;
		std::string pass;
		std::string path;
		std::string query;
		std::string fragment;
	};

	static Log errorLog;			// TODO To be removed
	static Log warningLog;			// TODO To be removed
	static Log debugLog;			// TODO To be removed
	static Log accessLog;			// TODO To be removed

	//! Encodes string using URL-encoding
	static std::string encodeUrl(const std::string &url);
	//! Decodes string using URL-encoding
	static std::string decodeUrl(const std::string &encodedUrl);
	static Uri parseUri(const std::string& uriStr);
	static std::string composeUri(const Uri& uri);
	static Get parseGet(const std::string& getStr);
	static std::string composeGet(const Get& get);
	static Post parsePost(const std::string& postStr);
	static std::string composePost(const Post& get);
};

} // namespace isl

#endif

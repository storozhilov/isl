#ifndef ISL__HTTP__HXX
#define ISL__HTTP__HXX

#include <isl/Log.hxx>		// TODO Remove it
#include <map>

//! TODO Remove all rocket science from the HTTP subsystem!!!

namespace isl
{

typedef std::multimap<std::string, std::string> HttpHeader;
typedef std::map<std::string, std::string> HttpCookies;

// TODO Remove it
class Http
{
public:
	static Log errorLog;
	static Log warningLog;
	static Log debugLog;
	static Log accessLog;
};

} // namespace isl

#endif

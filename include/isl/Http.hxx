#ifndef ISL__HTTP__HXX
#define ISL__HTTP__HXX

#include <isl/Log.hxx>

namespace isl
{

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

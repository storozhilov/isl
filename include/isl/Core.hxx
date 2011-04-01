#ifndef ISL__CORE__HXX
#define ISL__CORE__HXX

#include <isl/Log.hxx>

namespace isl
{

class Core
{
public:
	static void daemonize();
	static void writePid(const char * pidFileName);

	static Log errorLog;
	static Log warningLog;
	static Log debugLog;
};

} // namespace isl

#endif

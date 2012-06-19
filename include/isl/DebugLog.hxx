#ifndef ISL__DEBUG_LOG__HXX
#define ISL__DEBUG_LOG__HXX

#include <isl/Log.hxx>

namespace isl
{

//! Debug log class which log() method does nothing if ISL_CORE_DEBUGGING macro is undefined
class DebugLog : public Log
{
public:
	DebugLog();
	DebugLog(const std::string& prefix);
	DebugLog(const std::string& prefix, bool composeSourceLocation);

	void log(const AbstractLogMessage& msg);
private:
	DebugLog(const Log&);							// No copy

	DebugLog& operator=(const Log&);					// No copy
};

} // namespace isl

#endif

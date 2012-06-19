#include <isl/DebugLog.hxx>

namespace isl
{

DebugLog::DebugLog() :
	Log()
{}

DebugLog::DebugLog(const std::string& prefix) :
	Log(prefix)
{}

DebugLog::DebugLog(const std::string& prefix, bool composeSourceLocation) :
	Log(prefix, composeSourceLocation)
{}

void DebugLog::log(const AbstractLogMessage& msg)
{
#ifdef ISL_CORE_DEBUGGING
	Log::log(msg);
#endif
}

} // namespace isl

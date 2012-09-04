#include <isl/Log.hxx>

namespace isl
{

LogDispatcher Log::dispatcher;							// Static member initialization

Log::Log() :
	_prefix(),
	_copmposeSourceLocation(false)
{}

Log::Log(const std::string& prefix, bool composeSourceLocation) :
	_prefix(prefix),
	_copmposeSourceLocation(composeSourceLocation)
{}

Log::~Log()
{}

void Log::connectTarget(const AbstractLogTarget& target)
{
	dispatcher.connectLogToDevice(this, &target);
}

void Log::disconnectTarget(const AbstractLogTarget& target)
{
	dispatcher.disconnectLogFromDevice(this, &target);
}

void Log::disconnectTargets()
{
	dispatcher.disconnectLogFromDevices(this);
}

void Log::log(const AbstractLogMessage& msg)
{
	dispatcher.logMessage(this, msg);
}

} // namespace isl

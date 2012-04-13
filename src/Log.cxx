#include <isl/Log.hxx>
#include <iostream>

namespace isl
{

/*------------------------------------------------------------------------------
 * Log
 *----------------------------------------------------------------------------*/

LogDispatcher Log::dispatcher;							// Static member initialization

Log::Log() :
	_prefix(),
	_logRWLock()
{}

Log::Log(const std::wstring& prefix) :
	_prefix(prefix),
	_logRWLock()
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
	dispatcher.logMessage(this, msg.message());
}

void Log::log(const std::string& msg)
{
	dispatcher.logMessage(this, String::utf8Decode(msg));
}

void Log::log(const std::wstring& msg)
{
	dispatcher.logMessage(this, msg);
}

void Log::setPrefix(const std::wstring& newPrefix)
{
	WriteLocker locker(_logRWLock);
	_prefix = newPrefix;
}

std::wstring Log::prefix() const
{
	ReadLocker locker(_logRWLock);
	return _prefix;
}

std::wstring Log::composeSourceLocation(SOURCE_LOCATION_ARGS_DECLARATION)
{
	std::wostringstream sstr;
	sstr << String::utf8Decode(SOURCE_LOCATION_ARGS_FILE) << L'(' << SOURCE_LOCATION_ARGS_LINE << L"), " <<
		String::utf8Decode(SOURCE_LOCATION_ARGS_FUNCTION) << L": ";
	return sstr.str();
}

} // namespace isl

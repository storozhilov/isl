#include <isl/Log.hxx>
#include <isl/Utf8TextCodec.hxx>
#include <iostream>

namespace isl
{

/*------------------------------------------------------------------------------
 * Log
 *----------------------------------------------------------------------------*/

LogDispatcher Log::dispatcher;							// Static member initialization

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

void Log::logMessage(const std::wstring& msg)
{
	dispatcher.logMessage(this, msg);
}

void Log::logDebug(SOURCE_LOCATION_ARGS_DECLARATION, const std::wstring& msg)
{
	logMessage(composeSourceLocation(SOURCE_LOCATION_ARGS_PASSTHRU) + (msg.empty() ? std::wstring(L"[Empty message]") : msg));
}

void Log::logException(const std::exception& expt, SOURCE_LOCATION_ARGS_DECLARATION, const std::wstring& msg)
{
	std::wstring str = (dynamic_cast<const Exception *>(&expt)) ? L"isl::Exception instance has been thrown:\n" :
		L"std::exception instance has been thrown:\n";
	str += Utf8TextCodec().decode(expt.what());
	if (!msg.empty()) {
		str += L'\n';
		str += msg;
	}
	logDebug(SOURCE_LOCATION_ARGS_PASSTHRU, str);
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
	sstr << Utf8TextCodec().decode(SOURCE_LOCATION_ARGS_FILE) << L'(' << SOURCE_LOCATION_ARGS_LINE << L"), " <<
		Utf8TextCodec().decode(SOURCE_LOCATION_ARGS_FUNCTION) << L": ";
	return sstr.str();
}

} // namespace isl


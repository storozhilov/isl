#include "SourceBrowser.hxx"
#include <isl/Core.hxx>

namespace isl
{

SourceBrowser::SourceBrowser(AbstractSubsystem * owner, unsigned int workersAmount, unsigned int port, const std::wstring& rootPath) :
	AbstractSubsystem(owner),
	_taskDispatcher(this, workersAmount),
	_listener(this, _taskDispatcher, port, rootPath)
{}

void SourceBrowser::onStartCommand()
{
	setState<StartingState>();
	_taskDispatcher.start();
	_listener.start();
	setState<RunningState>();
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been started"));
}

void SourceBrowser::onStopCommand()
{
	setState<StoppingState>();
	_listener.stop();
	_taskDispatcher.stop();
	setState<IdlingState>();
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been stopped"));
}

} // namespace isl

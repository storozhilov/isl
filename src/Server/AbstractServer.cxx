#include <isl/AbstractServer.hxx>
#include <isl/Core.hxx>
#include <stdlib.h>
#include <pthread.h>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractServer
------------------------------------------------------------------------------*/

AbstractServer::AbstractServer(int argc, char * argv[]) :
	AbstractSubsystem(0),
	_argv()
{
	_argv.reserve(argc);
	for (int i = 0; i < argc; ++i) {
		_argv.push_back(argv[i]);
	}
}

void AbstractServer::run()
{
	setState<StartingState>();
	onStart();
	setState<RunningState>();
	while (true) {
		AbstractServer::State newState = awaitState();
		if (newState.equals<StoppingState>()) {
			Core::debugLog.logDebug(SOURCE_LOCATION_ARGS, L"Stopping state detected - stopping the server");
			onStop();
			setState<IdlingState>();
			Core::debugLog.logDebug(SOURCE_LOCATION_ARGS, L"Stopping server completed");
			break;
		} else if (newState.equals<RestartingState>()) {
			Core::debugLog.logDebug(SOURCE_LOCATION_ARGS, L"Restarting state detected - restarting the server");
			onRestart();
			setState<RunningState>();
			Core::debugLog.logDebug(SOURCE_LOCATION_ARGS, L"Restarting server completed");
		} else {
			Core::warningLog.logDebug(SOURCE_LOCATION_ARGS, L"Unexpected state '" + newState.value().name() +
					L"' detected - reverting to 'Running'");
			setState<RunningState>();
		}
	}
}

void AbstractServer::onRestart()
{
	onStop();
	onStart();
}

void AbstractServer::onStartCommand()
{
	Core::errorLog.logDebug(SOURCE_LOCATION_ARGS, L"Start command is not supported");
}

void AbstractServer::onStopCommand()
{
	setState<StoppingState>();
}

void AbstractServer::onRestartCommand()
{
	setState<RestartingState>();
}

} // namespace isl

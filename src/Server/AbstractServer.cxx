#include <isl/AbstractServer.hxx>
#include <isl/Core.hxx>
#include <stdlib.h>
#include <pthread.h>

namespace isl
{

AbstractServer::AbstractServer(int argc, char * argv[], const Timeout& timeout) :
	AbstractSubsystem(0),
	_argv(),
	_timeout(timeout)
{
	_argv.reserve(argc);
	for (int i = 0; i < argc; ++i) {
		_argv.push_back(argv[i]);
	}
}

void AbstractServer::run()
{
	start();
	while (true) {
		if (awaitState(_timeout) == IdlingState) {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Idling state detected - exiting from the main thread"));
			break;
		}

	}
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server has been stopped"));
}

} // namespace isl

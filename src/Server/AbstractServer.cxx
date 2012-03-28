#include <isl/AbstractServer.hxx>
#include <isl/Core.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractServer
------------------------------------------------------------------------------*/

AbstractServer::AbstractServer(int argc, char * argv[]) :
	AbstractSubsystem(0),
	_argv(),
	_commandsQueue(),
	_commandsCond()
{
	_argv.reserve(argc);
	for (int i = 0; i < argc; ++i) {
		_argv.push_back(argv[i]);
	}
}

void AbstractServer::run()
{
	beforeRun();
	start();
	MutexLocker locker(_commandsCond.mutex());
	while (true) {
		while (!_commandsQueue.empty()) {
			// Processing pending commands
			Command cmd = _commandsQueue.back();
			_commandsQueue.pop_back();
			switch (cmd) {
				case StartCommand:
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Start command received -> starting server"));
					start();
					break;
				case StopCommand:
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Stop command received -> stopping server"));
					stop();
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server has been stopped"));
					break;
				case ExitCommand:
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Exit command received -> stopping server"));
					stop();
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server has been stopped -> exiting"));
					afterRun();
					return;
				default:
					std::wostringstream msg;
					msg << L"Unknown server subsystem command :" << cmd;
					Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
		}
		_commandsCond.wait();
	}
}

void AbstractServer::sendCommand(Command cmd)
{
	MutexLocker locker(_commandsCond.mutex());
	if (_commandsQueue.size() >= MaxCommandQueueSize) {
		Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server commands queue overflow detected"));
	}
	_commandsQueue.push_front(cmd);
	_commandsCond.wakeOne();
}

} // namespace isl

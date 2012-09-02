#include <isl/common.hxx>
#include <isl/AbstractServer.hxx>
#include <isl/LogMessage.hxx>

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
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Start command received -> starting server"));
					start();
					break;
				case StopCommand:
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Stop command received -> stopping server"));
					stop();
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been stopped"));
					break;
				case ExitCommand:
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Exit command received -> stopping server"));
					stop();
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been stopped -> exiting"));
					afterRun();
					return;
				default:
					std::ostringstream msg;
					msg << "Unknown server subsystem command :" << cmd;
					errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
		}
		_commandsCond.wait();
	}
}

void AbstractServer::sendCommand(Command cmd)
{
	MutexLocker locker(_commandsCond.mutex());
	if (_commandsQueue.size() >= MaxCommandQueueSize) {
		errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server commands queue overflow detected"));
	}
	_commandsQueue.push_front(cmd);
	_commandsCond.wakeAll();
}

} // namespace isl

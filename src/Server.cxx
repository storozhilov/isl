#include <isl/common.hxx>
#include <isl/Server.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ErrorLogMessage.hxx>

namespace isl
{

Server::Server(int argc, char * argv[], const SignalSet& trackSignals, const Timeout& clockTimeout) :
	Subsystem(0),
	_argv(),
	_trackSignals(trackSignals),
	_clockTimeout(clockTimeout),
	_commandsQueue(),
	_initialSignalMask()
{
	_argv.reserve(argc);
	for (int i = 0; i < argc; ++i) {
		_argv.push_back(argv[i]);
	}
}

void Server::run()
{
	//! Firing before run event
	beforeRun();
	// Blocking UNIX-signals to be tracked
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Blocking UNIX-signals"));
	sigset_t blockedSignalMask = _trackSignals.sigset();
	if (pthread_sigmask(SIG_SETMASK, &blockedSignalMask, &_initialSignalMask)) {
		SystemCallError err(SOURCE_LOCATION_ARGS, SystemCallError::PThreadSigMask, errno, "Blocking UNIX-signals error");
		errorLog().log(ErrorLogMessage(SOURCE_LOCATION_ARGS, err));
		throw Exception(err);
	}
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "UNIX-signals have beed blocked"));
	// Starting children subsystems
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Starting server"));
	start();
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been started"));
	// Server's main loop
	while (true) {
		// Checkout for pending signals
		int pendingSignal = sigtimedwait(&blockedSignalMask, 0, &_clockTimeout.timeSpec());
		if (pendingSignal > 0) {
			// Pending signal has been fetched -> firing on signal event
			if (!onSignal(pendingSignal)) {
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS,
							"Server termintaion requested by the on signal event handler -> exiting from the server's main loop"));
				break;
			}
		} else if (errno != EAGAIN) {
			SystemCallError err(SOURCE_LOCATION_ARGS, SystemCallError::SigTimedWait, errno, "Error fetching pending signal");
			errorLog().log(ErrorLogMessage(SOURCE_LOCATION_ARGS, err));
			throw Exception(err);
		}
		// Checkout for pending commandswith zero timeout
		std::auto_ptr<Command> pendingCmdAutoPtr = _commandsQueue.pop(Timeout());
		if (pendingCmdAutoPtr.get()) {
			if (*pendingCmdAutoPtr.get() == RestartCommand) {
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Restart command received -> restarting server"));
				stop();
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been stopped"));
				start();
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been restarted"));
			} else if (*pendingCmdAutoPtr.get() == TerminateCommand) {
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Restart command received -> exiting from the server's main loop"));
			} else {
				std::ostringstream msg;
				msg << "Unknown server command has been received: " << *pendingCmdAutoPtr.get();
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
		}
	}
	// Stopping children subsystems
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Stopping server"));
	stop();
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been stopped"));
	// Restoring initial signal mask
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Unblocking UNIX-signals"));
	if (pthread_sigmask(SIG_SETMASK, &_initialSignalMask, 0)) {
		SystemCallError err(SOURCE_LOCATION_ARGS, SystemCallError::PThreadSigMask, errno, "Unblocking UNIX-signals error");
		errorLog().log(ErrorLogMessage(SOURCE_LOCATION_ARGS, err));
		throw Exception(err);
	}
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "UNIX-signals have beed unblocked"));
	//! Firing after run event
	afterRun();
}

bool Server::onSignal(int signo)
{
	std::ostringstream oss;
	oss << "Signal #" << signo << " has been received by server -> ";
	switch (signo) {
		case SIGHUP:
			oss << "restarting server";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
			stop();
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been stopped"));
			start();
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been restarted"));
			return true;
		case SIGINT:
		case SIGTERM:
			oss << "terminating server";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
			return false;
		default:
			oss << "no action defined";
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
			return true;
	}
}

} // namespace isl

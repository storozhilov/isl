#include <isl/Server.hxx>
#include <isl/common.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ErrorLogMessage.hxx>

namespace isl
{

Server::Server(int argc, char * argv[], const SignalSet& trackSignals, const Timeout& clockTimeout) :
	StateSetSubsystem(0, clockTimeout),
	_argv(),
	_trackSignals(trackSignals),
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
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "UNIX-signals have been blocked"));
	// Starting children subsystems
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Starting server"));
	start();
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been started"));
	// Server's main loop
	while (true) {
		// Handling pending signals. Code below has been commented cause sigtimedwait(2) system
		// call returns error when the computer recovers from hibernate mode
		/*int pendingSignal = sigtimedwait(&blockedSignalMask, 0, &clockTimeout().timeSpec());
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
		}*/
		// Handling pending signals
		if (hasPendingSignals()) {
			int pendingSignal = extractPendingSignal();
			/*std::ostringstream oss;
			oss << "Pending signal #" << pendingSignal << " detected";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));*/
			if (!onSignal(pendingSignal)) {
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS,
							"Server termintaion requested by the on signal event handler -> exiting from the server's main loop"));
				break;
			}
		}
		// Handling state
		StateSetType::SetType trackSet;
		trackSet.insert(TerminationState);
		trackSet.insert(RestartState);
		std::auto_ptr<StateSetType::SetType> setAutoPtr = stateSet().awaitAny(trackSet, Timestamp::limit(clockTimeout()));
		if (!setAutoPtr.get()) {
			continue;
		}
		if (setAutoPtr->find(TerminationState) != setAutoPtr->end()) {
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server termination has been detected -> exiting from the server's main loop"));
			break;
		} else if (setAutoPtr->find(RestartState) != setAutoPtr->end()) {
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server restart has been detected -> restarting the server"));
			restart();
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
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "UNIX-signals have been unblocked"));
	// Firing after run event
	afterRun();
}

void Server::appointRestart()
{
	stateSet().insert(RestartState);
}

void Server::restart()
{
	stop();
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been stopped"));
	start();
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been started"));
}

bool Server::onSignal(int signo)
{
	std::ostringstream oss;
	oss << "Signal #" << signo << " has been received by server -> ";
	switch (signo) {
		case SIGHUP:
			oss << "restarting server";
			restart();
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

bool Server::hasPendingSignals() const
{
	sigset_t pendingSignals;
	if (sigpending(&pendingSignals)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SigPending, errno));
	}
	for (std::set<int>::const_iterator i = _trackSignals.signals().begin();
			i != _trackSignals.signals().end(); ++i) {
		if (sigismember(&pendingSignals, *i)) {
			return true;
		}
	}
	return false;
}

int Server::extractPendingSignal() const
{
	sigset_t blockedSignalMask = _trackSignals.sigset();
	int pendingSignal;
	if (sigwait(&blockedSignalMask, &pendingSignal)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SigWait, errno));
	}
	return pendingSignal;
}

} // namespace isl

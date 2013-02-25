#include <isl/Server.hxx>
#include <isl/Ticker.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ErrorLogMessage.hxx>
#include <cstdlib>

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
	// Blocking UNIX-signals to be tracked
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Blocking UNIX-signals"));
	sigset_t blockedSignalMask = _trackSignals.sigset();
	if (pthread_sigmask(SIG_SETMASK, &blockedSignalMask, &_initialSignalMask)) {
		SystemCallError err(SOURCE_LOCATION_ARGS, SystemCallError::PThreadSigMask, errno, "Blocking UNIX-signals error");
		Log::error().log(ErrorLogMessage(SOURCE_LOCATION_ARGS, err));
		throw Exception(err);
	}
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "UNIX-signals have been blocked"));
	// Starting children subsystems
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Starting server"));
	start();
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been started"));
	// Firing on start event
	if (onStart()) {
		StateSetType::SetType currentStateSet = stateSet().fetch();
		Ticker ticker(clockTimeout());
		bool firstTick = true;
		while (true) {
			size_t ticksExpired;
			Timestamp nextTickLimit = ticker.tick(&ticksExpired);
			if (firstTick) {
				firstTick = false;
				currentStateSet = stateSet().fetch();
				if (currentStateSet.find(TerminationState) != currentStateSet.end()) {
					Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
								"Server termination has been detected on first tick -> exiting from the server's main loop"));
					break;
				}
			} else if (ticksExpired > 1) {
				// Overload has been detected
				Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Server execution overload has been detected: ") << ticksExpired << " ticks expired");
				if (!onOverload(ticksExpired, currentStateSet)) {
					Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been terminated by onOverload() event handler -> stopping server"));
				}
			}
			// Doing the job
			if (!doLoad(nextTickLimit, currentStateSet)) {
				Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been terminated by doLoad() method -> stopping server"));
				break;
			}
			// Inspecting for pending signals
			// NOTE: sigtimedwait(2) system call have not been used here, cause it returns an error when the computer recovers from hibernate mode
			if (hasPendingSignals()) {
				int pendingSignal = extractPendingSignal();
				if (!onSignal(pendingSignal)) {
					Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been terminated by onSignal() event handler -> stopping server"));
					break;
				}
			}
			// Awaiting for termination/restart
			StateSetType::SetType trackSet;
			trackSet.insert(TerminationState);
			trackSet.insert(RestartState);
			currentStateSet = stateSet().awaitAny(trackSet, nextTickLimit);
			if (currentStateSet.find(TerminationState) != currentStateSet.end()) {
				Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server termination has been detected -> exiting from the server's main loop"));
				break;
			} else if (currentStateSet.find(RestartState) != currentStateSet.end()) {
				Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server restart has been detected -> restarting the server"));
				restart();
			}
		}
	} else {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been terminated by onStart() event handler -> stopping server"));
	}
	// Stopping children subsystems
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Stopping server"));
	stop();
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been stopped"));
	// Firing on stop event
	onStop();
	// Restoring initial signal mask
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Unblocking UNIX-signals"));
	if (pthread_sigmask(SIG_SETMASK, &_initialSignalMask, 0)) {
		SystemCallError err(SOURCE_LOCATION_ARGS, SystemCallError::PThreadSigMask, errno, "Unblocking UNIX-signals error");
		Log::error().log(ErrorLogMessage(SOURCE_LOCATION_ARGS, err));
		throw Exception(err);
	}
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "UNIX-signals have been unblocked"));
}

void Server::appointRestart()
{
	stateSet().insert(RestartState);
}

void Server::daemonize()
{
	pid_t childPid = ::fork();
	if (childPid < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Fork, childPid));
	}
	if (childPid > 0) {
		exit(0);
	}
	pid_t newSessionId = ::setsid();
	if (newSessionId < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SetSid, newSessionId));
	}
}

void Server::restart()
{
	stop();
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been stopped"));
	start();
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been started"));
}

bool Server::onSignal(int signo)
{
	LogMessage msg(SOURCE_LOCATION_ARGS, "Signal #");
	msg << signo << " has been received by server -> ";
	switch (signo) {
		case SIGHUP:
			msg << "restarting server";
			Log::debug().log(msg);
			restart();
			return true;
		case SIGINT:
		case SIGTERM:
			msg << "terminating server";
			Log::debug().log(msg);
			return false;
		default:
			msg << "no action defined";
			Log::warning().log(msg);
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

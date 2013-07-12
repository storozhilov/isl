#include <isl/Server.hxx>
#include <isl/Ticker.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ErrorLogMessage.hxx>
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>

namespace isl
{

Server::Server(int argc, char * argv[], const SignalSet& trackSignals, const Timeout& clockTimeout) :
	Subsystem(0, clockTimeout),
	_argv(),
	_threadHandle(),
	_requester(),
	_trackSignals(trackSignals),
	_initialSignalMask(),
	_shouldRestart(false),
	_shouldTerminate(false)
{
	_argv.reserve(argc);
	for (int i = 0; i < argc; ++i) {
		_argv.push_back(argv[i]);
	}
}

void Server::run()
{
	_threadHandle = Thread::self();
	_shouldRestart = false;
	_shouldTerminate = false;
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
	onStart();
	// Init clock
	Ticker ticker(clockTimeout());
	bool firstTick = true;
	while (true) {
		// Handling termination flag
		if (_shouldTerminate) {
			// Termination state has been detected
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server termination has been detected -> terminating server"));
			break;
		}
		// Handling restart flag
		if (_shouldRestart) {
			// Restart has been detected
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server restart has been detected -> restarting the server"));
			stop();
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been stopped"));
			// Firing on stop event
			onStop();
			start();
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server has been started"));
			_shouldRestart = false;
			// Firing on start event
			onStart();
			firstTick = true;
		}
		// Making next clock tick
		size_t ticksExpired;
		Timestamp prevTickTimestamp;
		Timestamp nextTickTimestamp = ticker.tick(&ticksExpired, &prevTickTimestamp);
		if (firstTick) {
			firstTick = false;
		} else if (ticksExpired > 1) {
			// Overload has been detected
			Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Server execution overload has been detected: ") << ticksExpired << " ticks expired");
			onOverload(prevTickTimestamp, nextTickTimestamp, ticksExpired);
		}
		// Doing the job
		doLoad(prevTickTimestamp, nextTickTimestamp, ticksExpired);
		// Processing pending signals
		processSignals();
		// Awaiting for requests and processing them
		processRequests(nextTickTimestamp);
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
	if (_threadHandle == Thread::self()) {
		_shouldRestart = true;
		return;
	}
	// Sending restart request
	size_t requestId = _requester.sendRequest(RestartRequest());
	if (requestId <= 0) {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Could not send restart request to the thread requester thread"));
	} else {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Restart request has been sent to the thread requester thread"));
	}
	// Fetching the response
	Timestamp limit = Timestamp::limit(awaitResponseTimeout());
	std::auto_ptr<AbstractThreadMessage> responseAutoPtr = _requester.awaitResponse(requestId, limit);
	if (!responseAutoPtr.get()) {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS,
					"No response to restart request have been received from the thread requester thread"));
	} else if (responseAutoPtr->instanceOf<OkResponse>()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
					"OK response to the restart request has been received from the thread requester thread"));
	} else {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS,
					"Invalid response to restart request has been received from the thread requester thread: \"") <<
				responseAutoPtr->name() << '"');
	}
}

void Server::appointTermination()
{
	if (_threadHandle == Thread::self()) {
		_shouldTerminate = true;
		return;
	}
	// Sending termination request
	size_t requestId = _requester.sendRequest(TerminationRequest());
	if (requestId <= 0) {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Could not send termination request to the thread requester thread"));
	} else {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Termination request has been sent to the thread requester thread"));
	}
	// Fetching the response
	Timestamp limit = Timestamp::limit(awaitResponseTimeout());
	std::auto_ptr<AbstractThreadMessage> responseAutoPtr = _requester.awaitResponse(requestId, limit);
	if (!responseAutoPtr.get()) {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS,
					"No response to termination request have been received from the thread requester thread"));
	} else if (responseAutoPtr->instanceOf<OkResponse>()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
					"OK response to the termination request has been received from the thread requester thread"));
	} else {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS,
					"Invalid response to termination request has been received from the thread requester thread: \"") <<
				responseAutoPtr->name() << '"');
	}
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

void Server::onSignal(int signo)
{
	LogMessage msg(SOURCE_LOCATION_ARGS, "Signal #");
	msg << signo << " has been received by server -> ";
	switch (signo) {
		case SIGHUP:
			msg << "appointing a server restart";
			Log::debug().log(msg);
			appointRestart();
			break;
		case SIGINT:
		case SIGTERM:
			msg << "appointing a server termination";
			Log::debug().log(msg);
			appointTermination();
			break;
		default:
			msg << "no action defined";
			Log::warning().log(msg);
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

void Server::processSignals()
{
	while (hasPendingSignals()) {
		int pendingSignal = extractPendingSignal();
		onSignal(pendingSignal);
	}
}

std::auto_ptr<Subsystem::ThreadRequesterType::MessageType> Server::onRequest(const ThreadRequesterType::MessageType& request, bool responseRequired)
{
	Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Unrecognized request: '") << request.name() << '\'');
	return std::auto_ptr<ThreadRequesterType::MessageType>();
}

void Server::processRequests(const Timestamp& limit)
{
	while (const ThreadRequesterType::PendingRequest * pendingRequestPtr = _requester.awaitRequest(limit)) {
		std::auto_ptr<ThreadRequesterType::MessageType> responseAutoPtr = processRequest(pendingRequestPtr->request(), pendingRequestPtr->responseRequired());
		if (responseAutoPtr.get()) {
			_requester.sendResponse(*responseAutoPtr.get());
		} else if (pendingRequestPtr->responseRequired()) {
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "No response to the request, which requires one"));
		}
	}
}

std::auto_ptr<Subsystem::ThreadRequesterType::MessageType> Server::processRequest(const ThreadRequesterType::MessageType& request, bool responseRequired)
{
	if (request.instanceOf<TerminationRequest>()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
					"Termination request has been received by the thread requester thread -> setting the termination flag to TRUE"));
		_shouldTerminate = true;
		return std::auto_ptr<ThreadRequesterType::MessageType>(responseRequired ? new OkResponse() : 0);
	} else if (request.instanceOf<PingRequest>()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
					"Ping request has been received by the thread requester thread -> responding with the pong response"));
		return std::auto_ptr<ThreadRequesterType::MessageType>(responseRequired ? new PongResponse() : 0);
	}
	return onRequest(request, responseRequired);
}

} // namespace isl

#include <isl/common.hxx>
#include <isl/SignalHandler.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>
#include <errno.h>
#include <sstream>

namespace isl
{

/*------------------------------------------------------------------------------
 * SignalHandler
------------------------------------------------------------------------------*/

SignalHandler::SignalHandler(AbstractSubsystem * owner, const SignalSet& signalSet, const Timeout& timeout) :
	AbstractSubsystem(owner),
	_initialSignalMask(),
	_blockedSignals(signalSet),
	_timeout(timeout),
	_timeoutRWLock(),
	_signalHandlerThread(*this)
{}

void SignalHandler::onSignal(int signo)
{
	std::ostringstream oss;
	oss << "Signal #" << signo << " has been received by signal handler -> ";
	switch (signo) {
		case SIGHUP:
			oss << "sending restart command to server";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
			{
				AbstractServer * server = findServer();
				if (server) {
					server->doStop();
					server->doStart();
				} else {
					errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Instance of isl::AbstractServer not found in subsystems tree for restarting"));
				}
			}
			break;
		case SIGINT:
		case SIGTERM:
			oss << "sending exit command to server";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
			{
				AbstractServer * server = findServer();
				if (server) {
					server->doExit();
				} else {
					errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Instance of isl::AbstractServer not found in subsystems tree for exiting"));
				}
			}
			break;
		default:
			oss << "no action defined";
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
	}
}

AbstractServer * SignalHandler::findServer()
{
	AbstractSubsystem * curOwner = this;
	while ((curOwner = curOwner->owner())) {
		AbstractServer * server = dynamic_cast<AbstractServer *>(curOwner);
		if (server) {
			return server;
		}
	}
	return 0;
}

void SignalHandler::beforeStart()
{
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Starting signal handler"));
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Blocking signals"));
	sigset_t blockedSignalMask = _blockedSignals.sigset();
	if (pthread_sigmask(SIG_SETMASK, &blockedSignalMask, &_initialSignalMask)) {
		setState(IdlingState);
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadSigMask, errno));
	}
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Signals have been blocked"));
}

void SignalHandler::afterStart()
{
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Signal handler has been started"));
}

void SignalHandler::beforeStop()
{
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Stopping signal handler"));
}

void SignalHandler::afterStop()
{
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Unblocking signals"));
	if (pthread_sigmask(SIG_SETMASK, &_initialSignalMask, 0)) {
		// TODO Maybe to throw an exception?
		errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadSigMask, errno).message()));
	}
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Signals have been unblocked"));
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Signal handler has been stopped"));
}

/*------------------------------------------------------------------------------
 * SignalHandler::SignalHandlerThread
------------------------------------------------------------------------------*/

SignalHandler::SignalHandlerThread::SignalHandlerThread(SignalHandler& signalHandler) :
	SubsystemThread(signalHandler, true),
	_signalHandler(signalHandler)
{}

bool SignalHandler::SignalHandlerThread::hasPendingSignals() const
{
	sigset_t pendingSignals;
	if (sigpending(&pendingSignals)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SigPending, errno));
	}
	for (std::set<int>::const_iterator i = _signalHandler._blockedSignals.signals().begin();
			i != _signalHandler._blockedSignals.signals().end(); ++i) {
		if (sigismember(&pendingSignals, *i)) {
			return true;
		}
	}
	return false;
}

int SignalHandler::SignalHandlerThread::extractPendingSignal() const
{
	sigset_t blockedSignalMask = _signalHandler._blockedSignals.sigset();
	int pendingSignal;
	if (sigwait(&blockedSignalMask, &pendingSignal)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SigWait, errno));
	}
	return pendingSignal;
}

void SignalHandler::SignalHandlerThread::run()
{
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Signal handler thread has been started"));
	try {
		while (true) {
			if (hasPendingSignals()) {
				if (shouldTerminate()) {
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Signal handler termination detected before processing pending signals -> exiting from signal handler thread"));
					return;
				}
				while (hasPendingSignals()) {
					// Processing pending signals
					int pendingSignal = extractPendingSignal();
					std::ostringstream oss;
					oss << "Pending signal #" << pendingSignal << " detected";
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
					_signalHandler.onSignal(pendingSignal);
				}
			} else {
				if (awaitTermination(_signalHandler.timeout())) {
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Signal handler termination detected after inspecting for pending signals -> exiting from signal handler thread"));
					return;
				}
			}
		}
	} catch (std::exception& e) {
		errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Executing signal handler thread error -> exiting from signal handler thread"));
		_signalHandler.setState(AbstractSubsystem::IdlingState);
	} catch (...) {
		errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Executing signal handler thread unknown error -> exiting from signal handler thread"));
		_signalHandler.setState(AbstractSubsystem::IdlingState);
	}
}

} // namespace isl

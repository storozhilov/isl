#include <isl/SignalHandler.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <errno.h>
#include <iostream>
#include <sstream>

namespace isl
{

/*------------------------------------------------------------------------------
 * SignalHandler
------------------------------------------------------------------------------*/

SignalHandler::SignalHandler(AbstractSubsystem * owner, const SignalSet& signalSet, const Timeout& timeout) :
	AbstractSubsystem(owner),
	_startStopMutex(),
	_initialSignalMask(),
	_blockedSignals(signalSet),
	_timeout(timeout),
	_timeoutRWLock(),
	_signalHandlerThread(*this)
{}

Timeout SignalHandler::timeout() const
{
	ReadLocker locker(_timeoutRWLock);
	return _timeout;
}

void SignalHandler::setTimeout(const Timeout& newTimeout)
{
	WriteLocker locker(_timeoutRWLock);
	_timeout = newTimeout;
}

void SignalHandler::start()
{
	MutexLocker locker(_startStopMutex);
	setState(IdlingState, StartingState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Starting subsystem"));
	sigset_t blockedSignalMask = _blockedSignals.sigset();
	if (pthread_sigmask(SIG_SETMASK, &blockedSignalMask, &_initialSignalMask)) {
		setState(IdlingState);
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadSigMask, errno));
	}
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Signals have been blocked"));
	_signalHandlerThread.start();
}

void SignalHandler::stop()
{
	MutexLocker locker(_startStopMutex);
	setState(StoppingState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Stopping subsystem"));
	_signalHandlerThread.join();
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Signal handler thread has been stopped"));
	if (pthread_sigmask(SIG_SETMASK, &_initialSignalMask, 0)) {
		std::wcerr << SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadSigMask, errno).message() << std::endl;
	}
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Signals have been unblocked"));
	setState(IdlingState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been stopped"));
}

void SignalHandler::onSignal(int signo)
{
	std::wostringstream oss;
	oss << L"Signal #" << signo << L" has been received by signal handler -> ";
	switch (signo) {
		case SIGHUP:
			oss << L"restarting server";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
			{
				AbstractServer * server = findServer();
				if (server) {
					server->doStop();
					server->doStart();
				} else {
					Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Instance of isl::AbstractServer not found in subsystems tree for restarting"));
				}
			}
			break;
		case SIGINT:
		case SIGTERM:
			oss << L"stopping server";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
			{
				AbstractServer * server = findServer();
				if (server) {
					server->doExit();
				} else {
					Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Instance of isl::AbstractServer not found in subsystems tree for exiting"));
				}
			}
			break;
		default:
			oss << L"no action defined";
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
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

/*------------------------------------------------------------------------------
 * SignalHandler::SignalHandlerThread
------------------------------------------------------------------------------*/

SignalHandler::SignalHandlerThread::SignalHandlerThread(SignalHandler& signalHandler) :
	Thread(true),
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
	try {
		_signalHandler.setState(StartingState, RunningState);
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been started"));
		// Main subsystem's loop
		while (true) {
			if (hasPendingSignals()) {
				if (_signalHandler.state() != RunningState) {
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS,
							L"Signal handler is not in running state before processing pending signals -> exiting from signal handler thread"));
					break;
				}
				while (hasPendingSignals()) {
					// Processing pending signals
					int pendingSignal = extractPendingSignal();
					std::wostringstream oss;
					oss << "Pending signal #" << pendingSignal << " detected";
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
					_signalHandler.onSignal(pendingSignal);
				}
			} else {
				if (_signalHandler.awaitNotState(RunningState, _signalHandler._timeout) != RunningState) {
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS,
							L"Signal handler is not in running state after inspecting for pending signals -> exiting from signal handler thread"));
					break;
				}
			}
		}
	} catch (std::exception& e) {
		Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Executing signal handler thread error -> exiting from signal handler thread"));
		_signalHandler.setState(AbstractSubsystem::IdlingState);
	} catch (...) {
		Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Executing signal handler thread unknown error -> exiting from signal handler thread"));
		_signalHandler.setState(AbstractSubsystem::IdlingState);
	}
}

} // namespace isl

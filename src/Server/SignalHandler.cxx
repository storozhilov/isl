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

SignalHandler::SignalHandler(AbstractServer& server, const SignalSet& signalSet, const Timeout& timeout) :
	//AbstractSubsystem(&server),
	AbstractSubsystem(0),
	_server(server),
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

bool SignalHandler::start()
{
	setState(IdlingState, StartingState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Starting subsystem"));
	sigset_t blockedSignalMask = _blockedSignals.sigset();
	if (pthread_sigmask(SIG_SETMASK, &blockedSignalMask, &_initialSignalMask)) {
		setState(IdlingState);
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadSigMask, errno));
	}
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Signals have been blocked"));
	_signalHandlerThread.start();
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been started"));
	return true;
}

void SignalHandler::stop()
{
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

bool SignalHandler::restart()
{
	setState(StoppingState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Stopping subsystem during restart"));
	_signalHandlerThread.join();
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Signal handler thread has been stopped"));
	if (pthread_sigmask(SIG_SETMASK, &_initialSignalMask, 0)) {
		std::wcerr << SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadSigMask, errno).message() << std::endl;
	}
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Signals have been unblocked"));
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been stopped during restart"));
	setState(StoppingState, StartingState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Starting subsystem during restart"));
	sigset_t blockedSignalMask = _blockedSignals.sigset();
	if (pthread_sigmask(SIG_SETMASK, &blockedSignalMask, &_initialSignalMask)) {
		setState(IdlingState);
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadSigMask, errno));
	}
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Signals have been blocked"));
	_signalHandlerThread.start();
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been started during restart"));
	return true;
}

bool SignalHandler::onSignal(int signo)
{
	std::wostringstream oss;
	oss << L"Signal #" << signo << L" has been received by signal handler - ";
	switch (signo) {
		case SIGHUP:
			oss << L"restarting server";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
			_server.restart();
			return false;
		case SIGINT:
		case SIGTERM:
			oss << L"stopping server";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
			_server.stop();
			return false;
		default:
			oss << L"no action defined" << signo;
			Core::warningLog.log(oss.str());
			return true;
	}
}

/*------------------------------------------------------------------------------
 * SignalHandler::SignalHandlerThread
------------------------------------------------------------------------------*/

SignalHandler::SignalHandlerThread::SignalHandlerThread(SignalHandler& signalHandler) :
	Thread(true),
	_signalHandler(signalHandler),
	_timeoutCond()
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
		// Main subsystem's loop
		while (true) {
			// Checking out subsystem's state
			AbstractSubsystem::State signalHandlerState = _signalHandler.state();
			if (signalHandlerState != RunningState) {
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS,
						L"Signal handler is not in running state before inspecting pending signals. Stopping subsystem."));
				break;
			}
			if (!hasPendingSignals()) {
				// Sleeping using a wait condition and starting again
				MutexLocker locker(_timeoutCond.mutex());
				_timeoutCond.wait(_signalHandler._timeout);
				continue;
			}
			// Extracting pending signals
			std::list<int> pendingSignals;
			do {
				int pendingSignal = extractPendingSignal();
				std::wostringstream oss;
				oss << "Pending signal #" << pendingSignal << " detected";
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
				pendingSignals.push_back(pendingSignal);
			} while (hasPendingSignals());
			// Processing pending signals
			bool keepRunning = true;
			for (std::list<int>::iterator i = pendingSignals.begin(); i != pendingSignals.end(); ++i) {
				if (!_signalHandler.onSignal(*i)) {
					keepRunning = false;
					break;
				}
			}
			if (!keepRunning) {
				break;
			}
		}
	} catch (std::exception& e) {
		Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Executing signal handler thread error. Stopping subsystem."));
	} catch (...) {
		Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Executing signal handler thread unknown error. Stopping subsystem."));
	}
	//_signalHandler.setState(IdlingState);
}

} // namespace isl

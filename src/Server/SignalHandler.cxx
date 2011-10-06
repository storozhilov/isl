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

SignalHandler::SignalHandler(AbstractServer& server, const SignalSet& signalSet) :
	AbstractSubsystem(&server),
	_server(server),
	_initialSignalMask(),
	_blockedSignals(signalSet),
	_timeout(1),						// 1 second. TODO Use configuration subsystem
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

void SignalHandler::onStartCommand()
{
	setState<StartingState>();
	sigset_t blockedSignalMask = _blockedSignals.sigset();
	if (pthread_sigmask(SIG_SETMASK, &blockedSignalMask, &_initialSignalMask)) {
		setState<IdlingState>();
		throw Exception(SystemCallError(SystemCallError::PThreadSigMask, errno, SOURCE_LOCATION_ARGS));
	}
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Signals have been blocked in the main thread"));
	_signalHandlerThread.start();
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been started"));
}

void SignalHandler::onStopCommand()
{
	setState<StoppingState>();
	_signalHandlerThread.join();
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Signal handler thread has been stopped"));
	if (pthread_sigmask(SIG_SETMASK, &_initialSignalMask, 0)) {
		std::wcerr << SystemCallError(SystemCallError::PThreadSigMask, errno, SOURCE_LOCATION_ARGS).message() << std::endl;
	}
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Signals have been unblocked in the main thread"));
	setState<IdlingState>();
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been stopped"));
}

bool SignalHandler::onSignal(int signo)
{
	std::wostringstream oss;
	oss << L"Signal #" << signo << L" has been received by signal handler. ";
	switch (signo) {
		case SIGHUP:
			oss << L"Restarting server.";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
			_server.restart();
			return false;
		case SIGINT:
		case SIGTERM:
			oss << L"Stopping server.";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
			_server.stop();
			return false;
		default:
			oss << L"No action defined." << signo;
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
		throw Exception(SystemCallError(SystemCallError::SigPending, errno, SOURCE_LOCATION_ARGS));
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
		throw Exception(SystemCallError(SystemCallError::SigWait, errno, SOURCE_LOCATION_ARGS));
	}
	return pendingSignal;
}

void SignalHandler::SignalHandlerThread::run()
{
	try {
		_signalHandler.setState<RunningState>();
		// Main subsystem's loop
		while (true) {
			// Checking out subsystem's state
			AbstractSubsystem::State signalHandlerState = _signalHandler.state();
			if (!signalHandlerState.equals<AbstractSubsystem::RunningState>()) {
				if (signalHandlerState.equals<AbstractSubsystem::StoppingState>()) {
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS,
							L"Signal handler stopping detected before inspecting pending signals - exiting"));
					break;
				} else if (signalHandlerState.equals<AbstractSubsystem::RestartingState>()) {
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS,
							L"Signal handler restarting detected before inspecting pending signals - exiting"));
					break;
				} else {
					// TODO
					Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS,
							L"Signal handler unexpected state '" + signalHandlerState.value().name() + L"' detected before inspecting pending signals"));
				}
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
	_signalHandler.setState<IdlingState>();
}

} // namespace isl

#include <isl/AbstractTcpListener.hxx>
#include <isl/Core.hxx>
#include <isl/TcpSocket.hxx>
#include <memory>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractTcpListener
------------------------------------------------------------------------------*/

AbstractTcpListener::AbstractTcpListener(AbstractSubsystem * owner, TaskDispatcher& taskDispatcher, unsigned int port,
		const Timeout& timeout, const std::list<std::wstring>& interfaces) :
	AbstractSubsystem(owner),
	_taskDispatcher(taskDispatcher),
	_timeout(timeout),
	_timeoutRWLock(),
	_port(port),
	_interfaces(interfaces),
	_backLog(15),									// TODO	Use configuration or constructor parameter
	_listenerThread(*this)
{}

Timeout AbstractTcpListener::timeout() const
{
	ReadLocker locker(_timeoutRWLock);
	return _timeout;
}

void AbstractTcpListener::setTimeout(const Timeout& newTimeout)
{
	WriteLocker locker(_timeoutRWLock);
	_timeout = newTimeout;
}

void AbstractTcpListener::onStartCommand()
{
	setState<StartingState>();
	_listenerThread.start();
}

void AbstractTcpListener::onStopCommand()
{
	setState<StoppingState>();
	_listenerThread.join();
}

unsigned int AbstractTcpListener::port() const
{
	ReadLocker locker(_portRWLock);
	return _port;
}

void AbstractTcpListener::setPort(unsigned int newPort)
{
	WriteLocker locker(_portRWLock);
	_port = newPort;
}

std::list<std::wstring> AbstractTcpListener::interfaces() const
{
	ReadLocker locker(_interfacesRWLock);
	return _interfaces;
}

void AbstractTcpListener::setInterfaces(const std::list<std::wstring>& newInterfaces)
{
	WriteLocker locker(_interfacesRWLock);
	_interfaces = newInterfaces;
}

unsigned int AbstractTcpListener::backLog() const
{
	ReadLocker locker(_backLogRWLock);
	return _backLog;
}

void AbstractTcpListener::setBackLog(unsigned int newBackLog)
{
	WriteLocker locker(_backLogRWLock);
	_backLog = newBackLog;
}

/*------------------------------------------------------------------------------
 * AbstractTcpListener::ListenerThread
------------------------------------------------------------------------------*/

AbstractTcpListener::ListenerThread::ListenerThread(AbstractTcpListener& listener) :
	Thread(true),
	_listener(listener),
	_sleepCond()
{}

void AbstractTcpListener::ListenerThread::run()
{
	// Starting section
	TcpSocket serverSocket;
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been created"));
	while (true) {
		try {
			if (!_listener.isInState<StartingState>()) {
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS,
						L"Unexpected state detected while starting up. Exiting from subsystem's thread."));
				_listener.setState<IdlingState>();
				return;
			}
			serverSocket.open();
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been opened"));
			serverSocket.bind(_listener.port(), _listener.interfaces());
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been binded"));
			serverSocket.listen(_listener.backLog());
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been switched to the listening state"));
			break;
		} catch (std::exception& e) {
			Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Starting subsystem error."));
			sleep();
		} catch (...) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Starting subsystem unknown error."));
			sleep();
		}
	}
	_listener.setState<RunningState>();
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been successfully started"));
	// Running section
	while (true) {
		try {
			if (!keepRunning()) {
				break;
			}
			std::auto_ptr<TcpSocket> socketAutoPtr(serverSocket.accept(_listener.timeout()));
			if (!keepRunning()) {
				break;
			}
			if (!socketAutoPtr.get()) {
				// Accepting TCP-connection timeout expired
				continue;
			}
			AbstractTcpTask * task = _listener.createTask(socketAutoPtr.get());
			if (_listener._taskDispatcher.perform(task)) {
				socketAutoPtr.release();
			} else {
				Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Too many TCP-connection requests"));
			}
		} catch (std::exception& e) {
			Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Running subsystem error. Stopping subsystem."));
			break;
		} catch (...) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Running subsystem unknown error. Stopping subsystem."));
			break;
		}
	}
	_listener.setState<IdlingState>();
}

void AbstractTcpListener::ListenerThread::sleep()
{
	MutexLocker locker(_sleepCond.mutex());
	_sleepCond.wait(_listener.timeout());
}

bool AbstractTcpListener::ListenerThread::keepRunning()
{
	AbstractSubsystem::State listenerState = _listener.state();
	if (!listenerState.equals<AbstractSubsystem::RunningState>()) {
		if (listenerState.equals<AbstractSubsystem::StoppingState>()) {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Stopping state detected - exiting from subsystem's thread."));
			return false;
		} else if (listenerState.equals<AbstractSubsystem::IdlingState>()) {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Restarting state detected - exiting from subsystem's thread"));
			return false;
		} else {
			Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS,
					L"Unexpected subsystem's state '" + listenerState.value().name() + L"' detected. Reverting to 'Running'."));
			_listener.setState<RunningState>();
		}
	}
	return true;
}

} // namespace isl


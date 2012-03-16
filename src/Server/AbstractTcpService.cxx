#include <isl/AbstractTcpService.hxx>
#include <isl/Core.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/Error.hxx>
#include <memory>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractTcpService
------------------------------------------------------------------------------*/

AbstractTcpService::AbstractTcpService(AbstractSubsystem * owner, size_t maxClients, size_t maxTaskQueueOverflowSize) :
	AbstractSubsystem(owner),
	_startStopMutex(),
	_lastListenerId(0),
	_listeners(),
	_taskDispatcher(this, maxClients, maxTaskQueueOverflowSize)
{}

AbstractTcpService::~AbstractTcpService()
{
	resetListenersUnsafe();
}

unsigned int AbstractTcpService::addListener(unsigned int port, const Timeout& timeout, const std::list<std::string>& interfaces, unsigned int backLog)
{
	MutexLocker locker(_startStopMutex);
	if (state() != IdlingState) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, L"Listener could be added while subsystem's idling only"));
	}
	Listener * newListener = new Listener(*this, port, timeout, interfaces, backLog);
	_listeners.insert(Listeners::value_type(++_lastListenerId, newListener));
	return _lastListenerId;
}

void AbstractTcpService::updateListener(unsigned int id, unsigned int port, const Timeout& timeout, const std::list<std::string>& interfaces, unsigned int backLog)
{
	MutexLocker locker(_startStopMutex);
	if (state() != IdlingState) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, L"Listener could be removed while subsystem's idling only"));
	}
	Listeners::iterator pos = _listeners.find(id);
	if (pos == _listeners.end()) {
		std::wostringstream oss;
		oss << L"Listener (id = " << id << L") not found";
		Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
		return;
	}
	pos->second->_port = port;
	pos->second->_timeout = timeout;
	pos->second->_interfaces = interfaces;
	pos->second->_backLog = backLog;
}

void AbstractTcpService::removeListener(unsigned int id)
{
	MutexLocker locker(_startStopMutex);
	if (state() != IdlingState) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, L"Listener could be removed while subsystem's idling only"));
	}
	Listeners::iterator pos = _listeners.find(id);
	if (pos == _listeners.end()) {
		std::wostringstream oss;
		oss << L"Listener (id = " << id << L") not found";
		Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
		return;
	}
	delete pos->second;
	_listeners.erase(pos);
}

void AbstractTcpService::resetListeners()
{
	MutexLocker locker(_startStopMutex);
	if (state() != IdlingState) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, L"Listeners could be reset while subsystem's idling only"));
	}
	resetListenersUnsafe();
}

void AbstractTcpService::start()
{
	MutexLocker locker(_startStopMutex);
	setState(IdlingState, StartingState);
	_taskDispatcher.start();
	for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		i->second->start();
	}
	setState(RunningState);
}

void AbstractTcpService::stop()
{
	MutexLocker locker(_startStopMutex);
	setState(StoppingState);
	for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		i->second->stop();
	}
	_taskDispatcher.stop();
	setState(IdlingState);
}

/*------------------------------------------------------------------------------
 * AbstractTcpService::ListenerThread
------------------------------------------------------------------------------*/

AbstractTcpService::ListenerThread::ListenerThread(Listener * listener) :
	Thread(true),
	_listener(listener),
	_sleepCond()
{}

void AbstractTcpService::ListenerThread::run()
{
	// Starting section
	TcpSocket serverSocket;
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been created"));
	while (true) {
		try {
			if (_listener->state() != AbstractSubsystem::StartingState) {
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS,
						L"Unexpected listener state detected while starting up. Exiting from listener's thread."));
				return;
			}
			if (!serverSocket.isOpen()) {
				serverSocket.open();
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been opened"));
			}
			serverSocket.bind(_listener->_port, _listener->_interfaces);
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been binded"));
			serverSocket.listen(_listener->_backLog);
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
	// Updating listener state
	_listener->setState(StartingState, RunningState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been successfully started"));
	// Running section
	while (true) {
		try {
			if (_listener->state() != AbstractSubsystem::RunningState) {
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Listener is not in running state before accepting TCP-connection -> exiting from listener thread"));
				break;
			}
			std::auto_ptr<TcpSocket> socketAutoPtr(serverSocket.accept(_listener->_timeout));
			if (_listener->state() != AbstractSubsystem::RunningState) {
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Listener is not in running state after accepting TCP-connection -> exiting from listener thread"));
				break;
			}
			if (!socketAutoPtr.get()) {
				// Accepting TCP-connection timeout expired
				continue;
			}
			std::auto_ptr<AbstractTask> taskAutoPtr(_listener->_service.createTask(socketAutoPtr.get()));
			socketAutoPtr.release();
			if (_listener->_service._taskDispatcher.perform(taskAutoPtr.get())) {
				taskAutoPtr.release();
			} else {
				Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Too many TCP-connection requests"));
			}
		} catch (std::exception& e) {
			Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Running subsystem error -> exiting from listener thread"));
			_listener->setState(AbstractSubsystem::IdlingState);
			break;
		} catch (...) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Running subsystem unknown error -> exiting from listener thread"));
			_listener->setState(AbstractSubsystem::IdlingState);
			break;
		}
	}
}

/*------------------------------------------------------------------------------
 * AbstractTcpService::Listener
------------------------------------------------------------------------------*/

AbstractTcpService::Listener::Listener(AbstractTcpService& service, unsigned int port, const Timeout& timeout, const std::list<std::string>& interfaces, unsigned int backLog) :
	AbstractSubsystem(&service),
	_service(service),
	_port(port),
	_timeout(timeout),
	_interfaces(interfaces),
	_backLog(backLog),
	_listenerThread(this)
{}

void AbstractTcpService::Listener::start()
{
	setState(IdlingState, StartingState);
	_listenerThread.start();
}

void AbstractTcpService::Listener::stop()
{
	setState(StoppingState);
	_listenerThread.join();
	setState(IdlingState);
}

} // namespace isl

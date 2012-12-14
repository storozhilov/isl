#include <isl/AbstractSyncTcpService.hxx>

namespace isl
{

//------------------------------------------------------------------------------
// AbstractSyncTcpService
//------------------------------------------------------------------------------

AbstractSyncTcpService::AbstractSyncTcpService(Subsystem * owner, size_t maxClients, const Timeout& clockTimeout) :
	Subsystem(owner),
	_taskDispatcher(this, maxClients),
	_clockTimeout(clockTimeout),
	_lastListenerConfigId(),
	_listenerConfigs(),
	_listeners()
{}

AbstractSyncTcpService::~AbstractSyncTcpService()
{
	resetListenerThreads();
}

int AbstractSyncTcpService::addListener(const TcpAddrInfo& addrInfo, unsigned int backLog)
{
	ListenerConfig newListenerConf(addrInfo, backLog);
	_listenerConfigs.insert(ListenerConfigs::value_type(++_lastListenerConfigId, newListenerConf));
	return _lastListenerConfigId;
}

void AbstractSyncTcpService::updateListener(int id, const TcpAddrInfo& addrInfo, unsigned int backLog)
{
	ListenerConfigs::iterator pos = _listenerConfigs.find(id);
	if (pos == _listenerConfigs.end()) {
		std::ostringstream oss;
		oss << "Listener (id = " << id << ") not found";
		warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
		return;
	}
	pos->second.addrInfo = addrInfo;
	pos->second.backLog = backLog;
}

void AbstractSyncTcpService::removeListener(int id)
{
	ListenerConfigs::iterator pos = _listenerConfigs.find(id);
	if (pos == _listenerConfigs.end()) {
		std::ostringstream oss;
		oss << "Listener (id = " << id << ") not found";
		warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
		return;
	}
	_listenerConfigs.erase(pos);
}

void AbstractSyncTcpService::startImpl()
{
	// Creating listeners
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Creating listeners"));
	for (ListenerConfigs::const_iterator i = _listenerConfigs.begin(); i != _listenerConfigs.end(); ++i) {
		std::auto_ptr<ListenerThread> newListenerAutoPtr(new ListenerThread(*this, i->second.addrInfo, i->second.backLog));
		_listeners.push_back(newListenerAutoPtr.get());
		newListenerAutoPtr.release();
	}
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listeners have been created"));
	// Calling base class method
	Subsystem::startImpl();
}

void AbstractSyncTcpService::stopImpl()
{
	// Calling base class method
	Subsystem::stopImpl();
	// Diposing listeners
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Disposing listeners"));
	resetListenerThreads();
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listeners have been disposed"));
}

void AbstractSyncTcpService::resetListenerThreads()
{
	for (ListenersContainer::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		delete (*i);
	}
	_listeners.clear();
}

//------------------------------------------------------------------------------
// AbstractSyncTcpService::ListenerThread
//------------------------------------------------------------------------------

void AbstractSyncTcpService::ListenerThread::run()
{
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener thread has been started"));
	try {
		TcpSocket serverSocket;
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been created"));
		serverSocket.open();
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been opened"));
		serverSocket.bind(_addrInfo);
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been binded"));
		serverSocket.listen(_backLog);
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been switched to the listening state"));
		while (true) {
			if (shouldTerminate()) {
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener thread termination detected before accepting TCP-connection -> exiting from the listener thread"));
				break;
			}
			std::auto_ptr<TcpSocket> socketAutoPtr(serverSocket.accept(_service._clockTimeout));
			if (shouldTerminate()) {
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener thread termination detected after accepting TCP-connection -> exiting from the listener thread"));
				break;
			}
			if (!socketAutoPtr.get()) {
				// Accepting TCP-connection timeout expired
				continue;
			}
			std::ostringstream msg;
			msg << "TCP-connection has been received from " << socketAutoPtr.get()->remoteAddr().firstEndpoint().host << ':' <<
				socketAutoPtr.get()->remoteAddr().firstEndpoint().port;
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			std::auto_ptr<AbstractTask> taskAutoPtr(_service.createTask(*socketAutoPtr.get()));
			if (!taskAutoPtr.get()) {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "Task creation factory method returned zero pointer"));
			}
			socketAutoPtr.release();
			if (!_service._taskDispatcher.perform(taskAutoPtr, &AbstractTask::execute)) {
				warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Too many TCP-connection requests"));
				_service.onOverload(*taskAutoPtr.get());
			}
		}
	} catch (std::exception& e) {
		errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Synchronous TCP-service listener execution error -> exiting from listener thread"));
	} catch (...) {
		errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Synchronous TCP-service listener unknown execution error -> exiting from listener thread"));
	}
}

} // namespace isl

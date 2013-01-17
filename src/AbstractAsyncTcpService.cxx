#include <isl/AbstractAsyncTcpService.hxx>

namespace isl
{

//------------------------------------------------------------------------------
// AbstractAsyncTcpService
//------------------------------------------------------------------------------

AbstractAsyncTcpService::AbstractAsyncTcpService(Subsystem * owner, size_t maxClients, const Timeout& clockTimeout) :
	StateSetSubsystem(owner, clockTimeout),
	_taskDispatcher(this, maxClients * 2),
	_lastListenerConfigId(),
	_listenerConfigs(),
	_listeners()
{}

AbstractAsyncTcpService::~AbstractAsyncTcpService()
{
	resetListenerThreads();
}

int AbstractAsyncTcpService::addListener(const TcpAddrInfo& addrInfo, unsigned int backLog)
{
	ListenerConfig newListenerConf(addrInfo, backLog);
	_listenerConfigs.insert(ListenerConfigs::value_type(++_lastListenerConfigId, newListenerConf));
	return _lastListenerConfigId;
}

void AbstractAsyncTcpService::updateListener(int id, const TcpAddrInfo& addrInfo, unsigned int backLog)
{
	ListenerConfigs::iterator pos = _listenerConfigs.find(id);
	if (pos == _listenerConfigs.end()) {
		warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener (id = ") << id << ") not found");
		return;
	}
	pos->second.addrInfo = addrInfo;
	pos->second.backLog = backLog;
}

void AbstractAsyncTcpService::removeListener(int id)
{
	ListenerConfigs::iterator pos = _listenerConfigs.find(id);
	if (pos == _listenerConfigs.end()) {
		warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener (id = ") << id << ") not found");
		return;
	}
	_listenerConfigs.erase(pos);
}

void AbstractAsyncTcpService::start()
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
	StateSetSubsystem::start();
}

void AbstractAsyncTcpService::stop()
{
	// Calling base class method
	StateSetSubsystem::stop();
	// Diposing listeners
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Disposing listeners"));
	resetListenerThreads();
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listeners have been disposed"));
}

void AbstractAsyncTcpService::resetListenerThreads()
{
	for (ListenersContainer::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		delete (*i);
	}
	_listeners.clear();
}

//------------------------------------------------------------------------------
// AbstractAsyncTcpService::ListenerThread
//------------------------------------------------------------------------------

AbstractAsyncTcpService::ListenerThread::ListenerThread(AbstractAsyncTcpService& service, const TcpAddrInfo& addrInfo, unsigned int backLog) :
	Thread(service),
	_service(service),
	_addrInfo(addrInfo),
	_backLog(backLog),
	_serverSocket()
{}

bool AbstractAsyncTcpService::ListenerThread::onStart()
{
	try {
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener thread has been started"));
		_serverSocket.open();
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been opened"));
		_serverSocket.bind(_addrInfo);
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been binded"));
		_serverSocket.listen(_backLog);
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been switched to the listening state"));
		return true;
	} catch (std::exception& e) {
		errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Asynchronous TCP-service listener socket initialization error -> exiting from listener thread"));
		return false;
	} catch (...) {
		errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Asynchronous TCP-service listener unknown socket initialization error -> exiting from listener thread"));
		return false;
	}
}

bool AbstractAsyncTcpService::ListenerThread::doLoad(const Timestamp& limit, const StateSetType::SetType& stateSet)
{
	try {
		while (Timestamp::now() < limit) {
			std::auto_ptr<TcpSocket> socketAutoPtr(_serverSocket.accept(limit.leftTo()));
			if (!socketAutoPtr.get()) {
				// Accepting TCP-connection timeout expired
				return true;
			}
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "TCP-connection has been received from ") <<
					socketAutoPtr.get()->remoteAddr().firstEndpoint().host << ':' <<
					socketAutoPtr.get()->remoteAddr().firstEndpoint().port);
			std::auto_ptr<AbstractTask> taskAutoPtr(_service.createTask(*socketAutoPtr.get()));
			if (!taskAutoPtr.get()) {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "Task creation factory method returned zero pointer"));
			}
			socketAutoPtr.release();
			if (!_service._taskDispatcher.perform(taskAutoPtr, &AbstractTask::executeReceive, &AbstractTask::executeSend)) {
				warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Too many TCP-connection requests"));
				_service.onOverload(*taskAutoPtr.get());
			}
		}
		return true;
	} catch (std::exception& e) {
		errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Asynchronous TCP-service listener execution error -> exiting from listener thread"));
		return false;
	} catch (...) {
		errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Asynchronous TCP-service listener unknown execution error -> exiting from listener thread"));
		return false;
	}
}

} // namespace isl

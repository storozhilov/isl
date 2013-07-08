#include <isl/AbstractSyncTcpService.hxx>

namespace isl
{

//------------------------------------------------------------------------------
// AbstractSyncTcpService
//------------------------------------------------------------------------------

AbstractSyncTcpService::AbstractSyncTcpService(Subsystem * owner, size_t maxClients, const Timeout& clockTimeout) :
	Subsystem(owner, clockTimeout),
	_taskDispatcher(this, maxClients),
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
		Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener (id = ") << id << ") not found");
		return;
	}
	pos->second.addrInfo = addrInfo;
	pos->second.backLog = backLog;
}

void AbstractSyncTcpService::removeListener(int id)
{
	ListenerConfigs::iterator pos = _listenerConfigs.find(id);
	if (pos == _listenerConfigs.end()) {
		Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener (id = ") << id << ") not found");
		return;
	}
	_listenerConfigs.erase(pos);
}

void AbstractSyncTcpService::start()
{
	// Creating listeners
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Creating listeners"));
	for (ListenerConfigs::const_iterator i = _listenerConfigs.begin(); i != _listenerConfigs.end(); ++i) {
		std::auto_ptr<ListenerThread> newListenerAutoPtr(createListener(i->second.addrInfo, i->second.backLog));
		_listeners.push_back(newListenerAutoPtr.get());
		newListenerAutoPtr.release();
	}
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Listeners have been created"));
	// Calling base class method
	Subsystem::start();
}

void AbstractSyncTcpService::stop()
{
	// Calling base class method
	Subsystem::stop();
	// Diposing listeners
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Disposing listeners"));
	resetListenerThreads();
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Listeners have been disposed"));
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

AbstractSyncTcpService::ListenerThread::ListenerThread(AbstractSyncTcpService& service, const TcpAddrInfo& addrInfo, unsigned int backLog) :
	RequesterThread(service),
	_service(service),
	_addrInfo(addrInfo),
	_backLog(backLog),
	_serverSocket()
{}

bool AbstractSyncTcpService::ListenerThread::onStart()
{
	try {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener thread has been started"));
		_serverSocket.open();
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been opened"));
		_serverSocket.bind(_addrInfo);
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been binded"));
		_serverSocket.listen(_backLog);
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been switched to the listening state"));
		return true;
	} catch (std::exception& e) {
		Log::error().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Synchronous TCP-service listener socket initialization error -> exiting from listener thread"));
		return false;
	} catch (...) {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Synchronous TCP-service listener unknown socket initialization error -> exiting from listener thread"));
		return false;
	}
}

bool AbstractSyncTcpService::ListenerThread::doLoad(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
{
	try {
		while (Timestamp::now() < nextTickTimestamp) {
			std::auto_ptr<TcpSocket> socketAutoPtr(_serverSocket.accept(nextTickTimestamp.leftTo()));
			if (!socketAutoPtr.get()) {
				// Accepting TCP-connection timeout expired
				return true;
			}
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "TCP-connection has been received from ") <<
					socketAutoPtr.get()->remoteAddr().firstEndpoint().host << ':' <<
					socketAutoPtr.get()->remoteAddr().firstEndpoint().port);
			std::auto_ptr<AbstractTask> taskAutoPtr(_service.createTask(*socketAutoPtr.get()));
			if (!taskAutoPtr.get()) {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "Task creation factory method returned zero pointer"));
			}
			socketAutoPtr.release();
			if (!_service._taskDispatcher.perform(taskAutoPtr, &AbstractTask::execute)) {
				Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Too many TCP-connection requests"));
				_service.onOverload(*taskAutoPtr.get());
			}
		}
		return true;
	} catch (std::exception& e) {
		Log::error().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Synchronous TCP-service listener execution error -> exiting from listener thread"));
		return false;
	} catch (...) {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Synchronous TCP-service listener unknown execution error -> exiting from listener thread"));
		return false;
	}
}

} // namespace isl

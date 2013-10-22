#include <isl/AbstractAsyncTcpService.hxx>

namespace isl
{

//------------------------------------------------------------------------------
// AbstractAsyncTcpService
//------------------------------------------------------------------------------

AbstractAsyncTcpService::AbstractAsyncTcpService(Subsystem * owner, size_t maxClients, const Timeout& clockTimeout) :
	Subsystem(owner, clockTimeout),
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
		Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener (id = ") << id << ") not found");
		return;
	}
	pos->second.addrInfo = addrInfo;
	pos->second.backLog = backLog;
}

void AbstractAsyncTcpService::removeListener(int id)
{
	ListenerConfigs::iterator pos = _listenerConfigs.find(id);
	if (pos == _listenerConfigs.end()) {
		Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener (id = ") << id << ") not found");
		return;
	}
	_listenerConfigs.erase(pos);
}

void AbstractAsyncTcpService::start()
{
	// Creating listeners
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Creating listeners"));
	for (ListenerConfigs::const_iterator i = _listenerConfigs.begin(); i != _listenerConfigs.end(); ++i) {
		std::auto_ptr<ListenerThread> newListenerAutoPtr(new ListenerThread(*this, i->second.addrInfo, i->second.backLog));
		_listeners.push_back(newListenerAutoPtr.get());
		newListenerAutoPtr.release();
	}
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Listeners have been created"));
	// Calling base class method
	Subsystem::start();
}

void AbstractAsyncTcpService::stop()
{
	// Calling base class method
	Subsystem::stop();
	// Diposing listeners
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Disposing listeners"));
	resetListenerThreads();
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Listeners have been disposed"));
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
	OscillatorThread(service),
	_service(service),
	_addrInfo(addrInfo),
	_backLog(backLog),
	_serverSocket()
{}

void AbstractAsyncTcpService::ListenerThread::onStart()
{
	try {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener thread has been started"));
		_serverSocket.open();
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been opened"));
		_serverSocket.bind(_addrInfo);
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been binded to ") <<
                                _addrInfo.firstEndpoint().host << ':' << _addrInfo.firstEndpoint().port << " endpoint");
		_serverSocket.listen(_backLog);
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been switched to the listening state"));
	} catch (std::exception& e) {
		Log::error().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Asynchronous TCP-service listener socket initialization error -> exiting from listener thread"));
		appointTermination();
	} catch (...) {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Asynchronous TCP-service listener unknown socket initialization error -> exiting from listener thread"));
		appointTermination();
	}
}

void AbstractAsyncTcpService::ListenerThread::doLoad(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
{
	try {
		while (Timestamp::now() < nextTickTimestamp) {
			std::auto_ptr<TcpSocket> socketAutoPtr(_serverSocket.accept(nextTickTimestamp.leftTo()));
			if (!socketAutoPtr.get()) {
				// Accepting TCP-connection timeout expired
				return;
			}
                        if (!_service.onConnected(*socketAutoPtr.get())) {
                                Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
                                                        "New connection has been rejected by onConnected() event handler -> dropping the client"));
                                continue;
                        }
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "TCP-connection has been received from ") <<
					socketAutoPtr.get()->remoteAddr().firstEndpoint().host << ':' <<
					socketAutoPtr.get()->remoteAddr().firstEndpoint().port);
			std::auto_ptr<AbstractTask> taskAutoPtr(_service.createTask(*socketAutoPtr.get()));
			if (!taskAutoPtr.get()) {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "Task creation factory method returned zero pointer"));
			}
			socketAutoPtr.release();
			if (!_service._taskDispatcher.perform(taskAutoPtr, &AbstractTask::executeReceive, &AbstractTask::executeSend)) {
				Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Too many TCP-connection requests"));
				_service.onOverload(*taskAutoPtr.get());
			}
		}
	} catch (std::exception& e) {
		Log::error().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Asynchronous TCP-service listener execution error -> exiting from listener thread"));
		appointTermination();
	} catch (...) {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Asynchronous TCP-service listener unknown execution error -> exiting from listener thread"));
		appointTermination();
	}
}

} // namespace isl

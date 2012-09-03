#include <isl/AbstractTcpService.hxx>

namespace isl
{

//------------------------------------------------------------------------------
// AbstractTcpService
//------------------------------------------------------------------------------

AbstractTcpService::AbstractTcpService(Subsystem * owner, size_t workersAmount, size_t maxTaskQueueOverflowSize) :
	Subsystem(owner),
	_lastListenerConfigId(0),
	_listenerConfigs(),
	_listenerConfigsRwLock(),
	_listeners(),
	_taskDispatcher(this, workersAmount, maxTaskQueueOverflowSize)
{}

AbstractTcpService::~AbstractTcpService()
{
	resetListenerThreads();
}

unsigned int AbstractTcpService::addListener(const TcpAddrInfo& addrInfo, const Timeout& listenTimeout, unsigned int backLog)
{
	ListenerConfig newListenerConf(addrInfo, listenTimeout, backLog);
	WriteLocker locker(_listenerConfigsRwLock);
	_listenerConfigs.insert(ListenerConfigs::value_type(++_lastListenerConfigId, newListenerConf));
	return _lastListenerConfigId;
}

void AbstractTcpService::updateListener(unsigned int id, const TcpAddrInfo& addrInfo, const Timeout& listenTimeout, unsigned int backLog)
{
	WriteLocker locker(_listenerConfigsRwLock);
	ListenerConfigs::iterator pos = _listenerConfigs.find(id);
	if (pos == _listenerConfigs.end()) {
		std::ostringstream oss;
		oss << "Listener (id = " << id << ") not found";
		warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
		return;
	}
	pos->second.addrInfo = addrInfo;
	pos->second.listenTimeout = listenTimeout;
	pos->second.backLog = backLog;
}

void AbstractTcpService::removeListener(unsigned int id)
{
	WriteLocker locker(_listenerConfigsRwLock);
	ListenerConfigs::iterator pos = _listenerConfigs.find(id);
	if (pos == _listenerConfigs.end()) {
		std::ostringstream oss;
		oss << "Listener (id = " << id << ") not found";
		warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
		return;
	}
	_listenerConfigs.erase(pos);
}

void AbstractTcpService::beforeStart()
{
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Creating listeners"));
	{
		ReadLocker locker(_listenerConfigsRwLock);
		for (ListenerConfigs::const_iterator i = _listenerConfigs.begin(); i != _listenerConfigs.end(); ++i) {
			std::auto_ptr<AbstractListenerThread> newListenerAutoPtr(createListener(i->second.addrInfo, i->second.listenTimeout, i->second.backLog));
			_listeners.push_back(newListenerAutoPtr.get());
			newListenerAutoPtr.release();
		}
	}
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listeners have been created"));
}

void AbstractTcpService::afterStop()
{
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Destroying listeners"));
	resetListenerThreads();
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listeners have been destroyed"));
}

//------------------------------------------------------------------------------
// AbstractTcpService::AbstractListenerThread
//------------------------------------------------------------------------------

} // namespace isl

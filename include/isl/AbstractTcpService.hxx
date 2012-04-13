#ifndef ISL__ABSTRACT_TCP_SERVICE__HXX
#define ISL__ABSTRACT_TCP_SERVICE__HXX

#include <isl/AbstractSubsystem.hxx>
#include <isl/Timeout.hxx>
#include <isl/TaskDispatcher.hxx>
#include <isl/TcpAddrInfo.hxx>
#include <isl/TcpSocket.hxx>
#include <map>
#include <list>
#include <memory>

namespace isl
{

//! Base class for the TCP-service implementation
class AbstractTcpService : public AbstractSubsystem
{
public:
//	typedef BasicTaskDispatcher::AbstractTask AbstractTaskType;
//	typedef BasicTaskDispatcher<AbstractTaskType> TaskDispatcherType;

	//! Constructor
	/*!
	  \param owner Pointer to the owner subsystem
	  \param workersAmount Workers amount in task dispatcher
	  \param maxTaskQueueOverflowSize Maximum tasks queue overflow size
	*/
	AbstractTcpService(AbstractSubsystem * owner, size_t workersAmount, size_t maxTaskQueueOverflowSize = 0) :
		AbstractSubsystem(owner),
		_lastListenerConfigId(0),
		_listenerConfigs(),
		_listenerConfigsRwLock(),
		_listeners(),
		_taskDispatcher(this, workersAmount, maxTaskQueueOverflowSize)
	{}
	//! Desctructor
	virtual ~AbstractTcpService()
	{
		resetListenerThreads();
	}

	//! Adds listener to the service
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param addrInfo TCP-address info to bind to
	  \param listenTimeout Timeout to wait for incoming connections
	  \param backLog Listen backlog
	  \return Listener id
	*/
	unsigned int addListener(const TcpAddrInfo& addrInfo, const Timeout& listenTimeout = Timeout::defaultTimeout(), unsigned int backLog = 15)
	{
		ListenerConfig newListenerConf(addrInfo, listenTimeout, backLog);
		WriteLocker locker(_listenerConfigsRwLock);
		_listenerConfigs.insert(ListenerConfigs::value_type(++_lastListenerConfigId, newListenerConf));
		return _lastListenerConfigId;
	}
	//! Updates listener
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param id Listener id
	  \param addrInfo TCP-address info to bind to
	  \param listenTimeout Timeout to wait for incoming connections
	  \param backLog Listen backlog
	*/
	void updateListener(unsigned int id, const TcpAddrInfo& addrInfo, const Timeout& listenTimeout = Timeout::defaultTimeout(), unsigned int backLog = 15)
	{
		WriteLocker locker(_listenerConfigsRwLock);
		ListenerConfigs::iterator pos = _listenerConfigs.find(id);
		if (pos == _listenerConfigs.end()) {
			std::wostringstream oss;
			oss << L"Listener (id = " << id << L") not found";
			Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
			return;
		}
		pos->second.addrInfo = addrInfo;
		pos->second.listenTimeout = listenTimeout;
		pos->second.backLog = backLog;
	}
	//! Removes listener
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param id Id of the listener to remove
	*/
	void removeListener(unsigned int id)
	{
		WriteLocker locker(_listenerConfigsRwLock);
		ListenerConfigs::iterator pos = _listenerConfigs.find(id);
		if (pos == _listenerConfigs.end()) {
			std::wostringstream oss;
			oss << L"Listener (id = " << id << L") not found";
			Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
			return;
		}
		_listenerConfigs.erase(pos);
	}
	//! Resets all listeners
	/*!
	  Subsystem's restart needed to actually apply new value
	*/
	void resetListeners()
	{
		WriteLocker locker(_listenerConfigsRwLock);
		_listenerConfigs.clear();
	}
	//! Thread-safely returns maximum task queue overflow size.
	inline size_t maxTaskQueueOverflowSize() const
	{
		return _taskDispatcher.maxTaskQueueOverflowSize();
	}
	//! Thread-safely sets the new maximum task queue overflow size.
	/*!
	  Changes will take place on the next task performing operation
	  \param newValue New maximum task queue overflow size
	*/
	inline void setMaxTaskQueueOverflowSize(size_t newValue)
	{
		_taskDispatcher.setMaxTaskQueueOverflowSize(newValue);
	}
protected:
	typedef AbstractTask AbstractTaskType;
	typedef BasicTaskDispatcher<AbstractTaskType> TaskDispatcherType;

	//! Base class for TCP-listener thread
	class AbstractListenerThread : public SubsystemThread
	{
	public:
		//! Constructor
		/*!
		  \param service Reference to TCP-service object
		  \param addrInfo TCP-address info to bind to
		  \param listenTimeout Timeout to wait for incoming connections
		  \param backLog Listen backlog
		*/
		AbstractListenerThread(AbstractTcpService& service, const TcpAddrInfo& addrInfo, const Timeout& listenTimeout, unsigned int backLog) :
			SubsystemThread(service, true),
			_service(service),
			_addrInfo(addrInfo),
			_backLog(backLog),
			_listenTimeout(listenTimeout)
		{}
		//! Returns a reference to TCP-service subsystem
		inline AbstractTcpService& service()
		{
			return _service;
		}
		//! Returns addrinfo to bind to
		inline const TcpAddrInfo& addrInfo() const
		{
			return _addrInfo;
		}
		//! Returns listen backlog
		inline unsigned int backLog() const
		{
			return _backLog;
		}
		//! Returns timeout to wait for incoming connections
		inline const Timeout& listenTimeout() const
		{
			return _listenTimeout;
		}
	protected:
		//! Returns a reference to task dispatcher
		inline TaskDispatcherType& taskDispatcher()
		{
			return _service._taskDispatcher;
		}
		//! On start event handler
		virtual void onStart()
		{}
		//! On stop event handler
		virtual void onStop()
		{}
	private:
		AbstractListenerThread();
		AbstractListenerThread(const AbstractListenerThread&);								// No copy

		AbstractListenerThread& operator=(const AbstractListenerThread&);							// No copy

		AbstractTcpService& _service;
		const TcpAddrInfo _addrInfo;
		const unsigned int _backLog;
		const Timeout _listenTimeout;
	};


	//! Returns workers amount
	inline size_t workersAmount() const
	{
		return _taskDispatcher.workersAmount();
	}
	//! Sets new workers amount
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param newValue New workers amount
	*/
	inline void setWorkersAmount(size_t newValue)
	{
		_taskDispatcher.setWorkersAmount(newValue);
	}
	//! Creates listeners
	virtual void beforeStart()
	{
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Creating listeners"));
		{
			ReadLocker locker(_listenerConfigsRwLock);
			for (ListenerConfigs::const_iterator i = _listenerConfigs.begin(); i != _listenerConfigs.end(); ++i) {
				std::auto_ptr<AbstractListenerThread> newListenerAutoPtr(createListener(i->second.addrInfo, i->second.listenTimeout, i->second.backLog));
				_listeners.push_back(newListenerAutoPtr.get());
				newListenerAutoPtr.release();
			}
		}
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Listeners have been created"));
	}
	//! Destroys listeners
	virtual void afterStop()
	{
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Destroying listeners"));
		resetListenerThreads();
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Listeners have been destroyed"));
	}
	//! Creating listener factory method
	/*!
	  \param addrInfo TCP-address info to bind to
	  \param listenTimeout Timeout to wait for incoming connections
	  \param backLog Listen backlog
	  \return auto_ptr with new listener
	*/
	virtual std::auto_ptr<AbstractListenerThread> createListener(const TcpAddrInfo& addrInfo, const Timeout& listenTimeout, unsigned int backLog) = 0;
private:
	AbstractTcpService();
	AbstractTcpService(const AbstractTcpService&);						// No copy

	AbstractTcpService& operator=(const AbstractTcpService&);				// No copy

	struct ListenerConfig
	{
		ListenerConfig(const TcpAddrInfo& addrInfo, const Timeout& listenTimeout, unsigned int backLog) :
			addrInfo(addrInfo),
			listenTimeout(listenTimeout),
			backLog(backLog)
		{}

		TcpAddrInfo addrInfo;
		Timeout listenTimeout;
		unsigned int backLog;
	};
	typedef std::map<unsigned int, ListenerConfig> ListenerConfigs;
	typedef std::list<AbstractListenerThread *> Listeners;

	inline void resetListenerThreads()
	{
		for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
			delete (*i);
		}
		_listeners.clear();
	}

	unsigned int _lastListenerConfigId;
	ListenerConfigs _listenerConfigs;
	ReadWriteLock _listenerConfigsRwLock;
	Listeners _listeners;
	TaskDispatcherType _taskDispatcher;
};

} // namespace isl

#endif

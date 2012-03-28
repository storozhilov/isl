#ifndef ISL__ABSTRACT_TCP_SERVICE__HXX
#define ISL__ABSTRACT_TCP_SERVICE__HXX

#include <isl/AbstractSubsystem.hxx>
#include <isl/Timeout.hxx>
#include <isl/TaskDispatcher.hxx>
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
	  \param port TCP-port to bind to
	  \param listenTimeout Timeout to wait for incoming connections
	  \param interfaces Network intarfaces to bind to
	  \param backLog Listen backlog
	  \return Listener id
	*/
	unsigned int addListener(unsigned int port, const Timeout& listenTimeout = Timeout::defaultTimeout(), const std::list<std::string>& interfaces = std::list<std::string>(), unsigned int backLog = 15)
	{
		ListenerConfig newListenerConf;
		newListenerConf.port = port;
		newListenerConf.listenTimeout = listenTimeout;
		newListenerConf.interfaces = interfaces;
		newListenerConf.backLog = backLog;
		WriteLocker locker(_listenerConfigsRwLock);
		_listenerConfigs.insert(ListenerConfigs::value_type(++_lastListenerConfigId, newListenerConf));
		return _lastListenerConfigId;
	}
	//! Updates listener
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param id Listener id
	  \param port TCP-port to bind to
	  \param listenTimeout Timeout to wait for incoming connections
	  \param interfaces Network intarfaces to bind to
	  \param backLog Listen backlog
	*/
	void updateListener(unsigned int id, unsigned int port, const Timeout& listenTimeout = Timeout::defaultTimeout(), const std::list<std::string>& interfaces = std::list<std::string>(), unsigned int backLog = 15)
	{
		WriteLocker locker(_listenerConfigsRwLock);
		ListenerConfigs::iterator pos = _listenerConfigs.find(id);
		if (pos == _listenerConfigs.end()) {
			std::wostringstream oss;
			oss << L"Listener (id = " << id << L") not found";
			Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
			return;
		}
		pos->second.port = port;
		pos->second.listenTimeout = listenTimeout;
		pos->second.interfaces = interfaces;
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
	//! Base class for TCP-listener thread
	class AbstractListener : public SubsystemThread
	{
	public:
		//! Constructor
		/*!
		  \param service Reference to TCP-service object
		  \param port TCP-port to bind to
		  \param listenTimeout Timeout to wait for incoming connections
		  \param interfaces Network intarfaces to bind to
		  \param backLog Listen backlog
		*/
		AbstractListener(AbstractTcpService& service, unsigned int port, const Timeout& listenTimeout, const std::list<std::string>& interfaces, unsigned int backLog) :
			SubsystemThread(service, true),
			_service(service),
			_port(port),
			_interfaces(interfaces),
			_backLog(backLog),
			_listenTimeout(listenTimeout)
		{}
		//! Returns a reference to TCP-service subsystem
		inline AbstractTcpService& service()
		{
			return _service;
		}
		//! Returns port to bind to
		inline unsigned int port() const
		{
			return _port;
		}
		//! Returns interfaces to bind to
		inline const std::list<std::string>& interfaces() const
		{
			return _interfaces;
		}
		//! Returns listen backlog
		inline unsigned int backLog() const
		{
			return _backLog;
		}
		//! Returns timeout to wait for incoming connections
		inline Timeout listenTimeout() const
		{
			return _listenTimeout;
		}
	protected:
		//! Returns a reference to task dispatcher
		inline TaskDispatcher& taskDispatcher()
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
		AbstractListener();
		AbstractListener(const AbstractListener&);								// No copy

		AbstractListener& operator=(const AbstractListener&);							// No copy

		//virtual void run();

		AbstractTcpService& _service;
		unsigned int _port;
		std::list<std::string> _interfaces;
		unsigned int _backLog;
		Timeout _listenTimeout;
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
				AbstractListener * newListener = createListener(i->second.port, i->second.listenTimeout, i->second.interfaces, i->second.backLog);
				_listeners.push_back(newListener);
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
	  \param port TCP-port to bind to
	  \param listenTimeout Timeout to wait for incoming connections
	  \param interfaces Network interfaces to bind to
	  \param backLog Listen backlog
	  \return New listener
	*/
	virtual AbstractListener * createListener(unsigned int port, const Timeout& listenTimeout, const std::list<std::string>& interfaces, unsigned int backLog) = 0;
private:
	AbstractTcpService();
	AbstractTcpService(const AbstractTcpService&);						// No copy

	AbstractTcpService& operator=(const AbstractTcpService&);					// No copy

	struct ListenerConfig
	{
		unsigned int port;
		Timeout listenTimeout;
		std::list<std::string> interfaces;
		unsigned int backLog;
	};
	typedef std::map<unsigned int, ListenerConfig> ListenerConfigs;
	typedef std::list<AbstractListener *> Listeners;

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
	TaskDispatcher _taskDispatcher;
};

} // namespace isl

#endif


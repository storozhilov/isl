#ifndef ISL__ABSTRACT_TCP_SERVICE__HXX
#define ISL__ABSTRACT_TCP_SERVICE__HXX

#include <isl/common.hxx>
#include <isl/Subsystem.hxx>
#include <isl/Timeout.hxx>
#include <isl/TaskDispatcher.hxx>
#include <isl/TcpAddrInfo.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/LogMessage.hxx>
#include <map>
#include <list>
#include <memory>

namespace isl
{

//! Base class for the TCP-service implementation
class AbstractTcpService : public Subsystem
{
public:
	//! Constructor
	/*!
	  \param owner Pointer to the owner subsystem
	  \param workersAmount Workers amount in task dispatcher
	  \param maxTaskQueueOverflowSize Maximum tasks queue overflow size
	*/
	AbstractTcpService(Subsystem * owner, size_t workersAmount, size_t maxTaskQueueOverflowSize = 0);
	//! Desctructor
	virtual ~AbstractTcpService();
	//! Adds listener to the service
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param addrInfo TCP-address info to bind to
	  \param listenTimeout Timeout to wait for incoming connections
	  \param backLog Listen backlog
	  \return Listener id
	*/
	unsigned int addListener(const TcpAddrInfo& addrInfo, const Timeout& listenTimeout = Timeout::defaultTimeout(), unsigned int backLog = 15);
	//! Updates listener
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param id Listener id
	  \param addrInfo TCP-address info to bind to
	  \param listenTimeout Timeout to wait for incoming connections
	  \param backLog Listen backlog
	*/
	void updateListener(unsigned int id, const TcpAddrInfo& addrInfo, const Timeout& listenTimeout = Timeout::defaultTimeout(), unsigned int backLog = 15);
	//! Removes listener
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param id Id of the listener to remove
	*/
	void removeListener(unsigned int id);
	//! Resets all listeners
	/*!
	  Subsystem's restart needed to actually apply new value
	*/
	inline void resetListeners()
	{
		WriteLocker locker(runtimeParamsRWLock);
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
	typedef BasicTaskDispatcher<AbstractTask> TaskDispatcherType;		//!< Task dispatcher type

	//! Base class for TCP-listener thread
	class AbstractListenerThread : public Subsystem::AbstractThread
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
			AbstractThread(service),
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
	virtual void beforeStart();
	//! Destroys listeners
	virtual void afterStop();
	//! Creating listener factory method
	/*!
	  \param addrInfo TCP-address info to bind to
	  \param listenTimeout Timeout to wait for incoming connections
	  \param backLog Listen backlog
	  \return Pointer to new listener
	*/
	virtual AbstractListenerThread * createListener(const TcpAddrInfo& addrInfo, const Timeout& listenTimeout, unsigned int backLog) = 0;
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
	Listeners _listeners;
	TaskDispatcherType _taskDispatcher;
};

} // namespace isl

#endif

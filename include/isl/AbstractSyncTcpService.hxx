#ifndef ISL__ABSTRACT_SYNC_TCP_SERVICE__HXX
#define ISL__ABSTRACT_SYNC_TCP_SERVICE__HXX

#include <isl/common.hxx>
#include <isl/StateSetSubsystem.hxx>
#include <isl/TaskDispatcher.hxx>
#include <isl/TcpAddrInfo.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>

namespace isl
{

//! Base class for synchronous TCP-service, which reads from and writes to the client connection socket in the same thread
class AbstractSyncTcpService : public StateSetSubsystem
{
public:
	//! Synchronous TCP-service abstract task
	class AbstractTask
	{
	public:
		//! Constructor
		/*!
		  \param socket Reference to the client connection socket
		*/
		AbstractTask(TcpSocket& socket) :
			_socketAutoPtr(&socket)
		{}
		// Destructor
		virtual ~AbstractTask()
		{}
		//! Returns a reference to the client connection socket
		inline TcpSocket& socket()
		{
			return *_socketAutoPtr.get();
		}
		//! Task execution method
		/*!
		  \param taskDispatcher Reference to the task dispatcher subsystem
		*/
		inline void execute(TaskDispatcher<AbstractTask>& taskDispatcher)
		{
			executeImpl(taskDispatcher);
		}
	protected:
		//! Task execution abstract virtual method to override in subclasses
		/*!
		  \param taskDispatcher Reference to the task dispatcher subsystem
		*/
		virtual void executeImpl(TaskDispatcher<AbstractTask>& taskDispatcher) = 0;
	private:
		std::auto_ptr<TcpSocket> _socketAutoPtr;
	};
	//! Constructor
	/*!
	  \param owner Pointer to the owner subsystem
	  \param maxClients Maximum clients amount to serve at the same time
	  \param clockTimeout Subsystem's clock timeout
	*/
	AbstractSyncTcpService(Subsystem * owner, size_t maxClients, const Timeout& clockTimeout = Timeout::defaultTimeout());
	//! Destructor
	virtual ~AbstractSyncTcpService();

	//! Returns maximum clients amount
	inline size_t maxClients() const
	{
		return _taskDispatcher.workersAmount();
	}
	//! Sets maximum clients amount
	/*!
	  \param newValue New maximum clients amount

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	inline void setMaxClients(size_t newValue)
	{
		_taskDispatcher.setWorkersAmount(newValue);
	}
	//! Adds listener to the service
	/*!
	  \param addrInfo TCP-address info to bind to
	  \param backLog Listen backlog
	  \return Listener id

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	int addListener(const TcpAddrInfo& addrInfo, unsigned int backLog = 15);
	//! Updates listener
	/*!
	  \param id Listener id
	  \param addrInfo TCP-address info to bind to
	  \param backLog Listen backlog

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	void updateListener(int id, const TcpAddrInfo& addrInfo, unsigned int backLog = 15);
	//! Removes listener
	/*!
	  \param id Id of the listener to remove

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	void removeListener(int id);
	//! Resets all listeners
	/*!
	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	inline void resetListeners()
	{
		_listenerConfigs.clear();
	}
	//! Starting service method redefinition
	virtual void start();
	//! Stopping service method redefinition
	virtual void stop();
protected:
	//! On overload event handler
	/*!
	  \param task Reference to the unperformed task
	*/
	virtual void onOverload(AbstractTask& task)
	{}

	//! Task creation abstract virtual method to override in subclasses
	/*!
	  \param socket Reference to the client connection socket
	*/
	virtual AbstractTask * createTask(TcpSocket& socket) = 0;
private:
	struct ListenerConfig
	{
		ListenerConfig(const TcpAddrInfo& addrInfo, unsigned int backLog) :
			addrInfo(addrInfo),
			backLog(backLog)
		{}

		TcpAddrInfo addrInfo;
		unsigned int backLog;
	};
	typedef std::map<int, ListenerConfig> ListenerConfigs;

	class ListenerThread : public AbstractThread
	{
	public:
		ListenerThread(AbstractSyncTcpService& service, const TcpAddrInfo& addrInfo, unsigned int backLog) :
			AbstractThread(service),
			_service(service),
			_addrInfo(addrInfo),
			_backLog(backLog)
		{}
	private:
		ListenerThread();
		ListenerThread(const ListenerThread&);								// No copy

		ListenerThread& operator=(const ListenerThread&);						// No copy

		virtual void run();

		AbstractSyncTcpService& _service;
		const TcpAddrInfo _addrInfo;
		const unsigned int _backLog;
	};

	typedef std::list<ListenerThread *> ListenersContainer;

	void resetListenerThreads();

	TaskDispatcher<AbstractTask> _taskDispatcher;
	int _lastListenerConfigId;
	ListenerConfigs _listenerConfigs;
	ListenersContainer _listeners;
};

} // namespace isl

#endif

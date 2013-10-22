#ifndef ISL__ABSTRACT_SYNC_TCP_SERVICE__HXX
#define ISL__ABSTRACT_SYNC_TCP_SERVICE__HXX

#include <isl/Subsystem.hxx>
#include <isl/TaskDispatcher.hxx>
#include <isl/TcpAddrInfo.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>
#include <map>

namespace isl
{

//! Base class for synchronous TCP-service, which reads from and writes to the client connection socket in the same thread
class AbstractSyncTcpService : public Subsystem
{
public:
	class AbstractTask;
	typedef TaskDispatcher<AbstractTask> TaskDispatcherType;
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
		inline void execute(TaskDispatcherType& taskDispatcher)
		{
			executeImpl(taskDispatcher);
		}
	protected:
		//! Task execution abstract virtual method to override in subclasses
		/*!
		  \param taskDispatcher Reference to the task dispatcher subsystem
		*/
		virtual void executeImpl(TaskDispatcherType& taskDispatcher) = 0;
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
	class ListenerThread : public OscillatorThread
	{
	public:
		//! Constructs a listener
		/*!
		 * \param service Reference to synchronous TCP-service object
		 * \param addrInfo TCP-address info to bind to
		 * \param backLog Listen backlog
		 */
		ListenerThread(AbstractSyncTcpService& service, const TcpAddrInfo& addrInfo, unsigned int backLog);
	private:
		ListenerThread();
		ListenerThread(const ListenerThread&);								// No copy

		ListenerThread& operator=(const ListenerThread&);						// No copy

		virtual void onStart();
		virtual void doLoad(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired);

		AbstractSyncTcpService& _service;
		const TcpAddrInfo _addrInfo;
		const unsigned int _backLog;
		TcpSocket _serverSocket;
	};

	//! Creating listener thread virtual factory method
	/*!
	 * \param addrInfo TCP-address info to bind to
	 * \param backLog Listen backlog
	 * \return Pointer to new listener thread
	 */
	virtual ListenerThread * createListener(const TcpAddrInfo& addrInfo, unsigned int backLog)
	{
		return new ListenerThread(*this, addrInfo, backLog);
	}
	//! On overload event handler
	/*!
	  \param task Reference to the unperformed task
	*/
	virtual void onOverload(AbstractTask& task)
	{}
        //! On client connection event handler
        /*!
          \param socket Reference to client connection socket
          \return TRUE if to accept connection or FALSE otherwise
          \note Default implementation does nothing and returns TRUE
        */
        virtual bool onConnected(TcpSocket& socket)
        {
                return true;
        }

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

	typedef std::list<ListenerThread *> ListenersContainer;

	void resetListenerThreads();

	TaskDispatcherType _taskDispatcher;
	int _lastListenerConfigId;
	ListenerConfigs _listenerConfigs;
	ListenersContainer _listeners;
};

} // namespace isl

#endif

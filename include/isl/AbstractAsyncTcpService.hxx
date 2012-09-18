#ifndef ISL__ABSTRACT_ASYNC_TCP_SERVICE__HXX
#define ISL__ABSTRACT_ASYNC_TCP_SERVICE__HXX

#include <isl/AbstractTcpService.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>

namespace isl
{

//! Base class for asynchronous TCP-service, which reads from and writes data to socket in two different threads per client connection
class AbstractAsyncTcpService : public AbstractTcpService
{
public:
	//! Constructor
	/*!
	  \param owner Pointer to the owner subsystem
	  \param maxClients Maximum clients amount to serve at the same time
	  \param maxTaskQueueOverflowSize Maximum tasks queue overflow size
	*/
	AbstractAsyncTcpService(Subsystem * owner, size_t maxClients, size_t maxTaskQueueOverflowSize = 0) :
		AbstractTcpService(owner, maxClients * 2, maxTaskQueueOverflowSize)
	{}

	//! Returns maximum clients amount
	inline size_t maxClients() const
	{
		return workersAmount() / 2;
	}
	//! Sets maximum clients amount
	/*!
	  Subsystem's restart needed to completely apply the new value;

	  \param newValue New maximum clients amount
	*/
	inline void setMaxClients(size_t newValue)
	{
		setWorkersAmount(newValue * 2);
	}
protected:
	class AbstractReceiverTask;
	class AbstractSenderTask;
	//! Shared staff which is to be used by sender and receiver tasks concurrently
	/*!
	  Object of this class is to be thread-safely erased during sender's and receiver's destructor invocations. Feel free to subclass.
	*/
	class SharedStaff
	{
	public:
		//! Constructor
		/*!
		  \param socket Reference to the client connection socket
		*/
		SharedStaff(TcpSocket& socket) :
			runtimeParamsRwLock(),
			_socketAutoPtr(&socket),
			_refsCount(0),
			//_refsCountMutex(),
			_shouldTerminate(false),
			//_shouldTerminateRwLock(),
			_receiverTask(),
			_senderTask()
		{}
		//! Destructor
		virtual ~SharedStaff()
		{}
		//! Returns reference to the reciever task object
		inline AbstractReceiverTask& receiverTask()
		{
			return *_receiverTask;
		}
		//! Returns reference to the sender task object
		inline AbstractSenderTask& senderTask()
		{
			return *_senderTask;
		}
		//! Returns reference to the client connection socket
		inline TcpSocket& socket()
		{
			return *_socketAutoPtr.get();
		}
		//! Returns true if the shared staff object considers that serving sould be terminated
		inline bool shouldTerminate() const
		{
			//ReadLocker locker(_shouldTerminateRwLock);
			ReadLocker locker(runtimeParamsRwLock);
			return _shouldTerminate;
		}
		//! Initiates serving termination
		inline bool setShouldTerminate(bool newValue)
		{
			//WriteLocker locker(_shouldTerminateRwLock);
			WriteLocker locker(runtimeParamsRwLock);
			bool oldValue = _shouldTerminate;
			_shouldTerminate = newValue;
			return oldValue;
		}
		//! Shared staff initialization virtual method which is to be called after constructor's call
		virtual void init()
		{}
	protected:
		//! Shared staff's runtime parameters R/W-lock
		/*!
		  Use it for thread-safely locking of any of your shared staff's runtime parameter.
		  This memeber has been introduced in order to save system resources by using the same
		  R/W-lock for all runtime parameters of the shared staff.
		*/
		mutable ReadWriteLock runtimeParamsRwLock;
	private:
		SharedStaff();
		SharedStaff(const SharedStaff&);						// No copy

		SharedStaff& operator=(const SharedStaff&);					// No copy

		inline int incRef()
		{
			//MutexLocker locker(_refsCountMutex);
			WriteLocker locker(runtimeParamsRwLock);
			return ++_refsCount;
		}
		inline int decRef()
		{
			//MutexLocker locker(_refsCountMutex);
			WriteLocker locker(runtimeParamsRwLock);
			return --_refsCount;
		}

		std::auto_ptr<TcpSocket> _socketAutoPtr;
		int _refsCount;
		//Mutex _refsCountMutex;
		bool _shouldTerminate;
		//mutable ReadWriteLock _shouldTerminateRwLock;
		AbstractReceiverTask * _receiverTask;
		AbstractSenderTask * _senderTask;

		friend class AbstractReceiverTask;
		friend class AbstractSenderTask;
		friend class ListenerThread;
	};

	//! Asynchronous TCP-service listener thread. Feel free to subclass.
	class ListenerThread : public AbstractTcpService::AbstractListenerThread
	{
	public:
		//! Constructor
		/*!
		  \param service Reference to TCP-service object
		  \param addrInfo TCP-address info to bind to
		  \param listenTimeout Timeout to wait for incoming connections
		  \param backLog Listen backlog
		*/
		ListenerThread(AbstractAsyncTcpService& service, const TcpAddrInfo& addrInfo, const Timeout& listenTimeout, unsigned int backLog) :
			AbstractListenerThread(service, addrInfo, listenTimeout, backLog),
			_service(service)
		{}
	private:
		ListenerThread();
		ListenerThread(const ListenerThread&);								// No copy

		ListenerThread& operator=(const ListenerThread&);						// No copy

		virtual void run()
		{
			onStart();
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener thread has been started"));
			try {
				TcpSocket serverSocket;
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been created"));
				serverSocket.open();
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been opened"));
				serverSocket.bind(addrInfo());
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been binded"));
				serverSocket.listen(backLog());
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Server socket has been switched to the listening state"));
				while (true) {
					if (shouldTerminate()) {
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener thread termination detected before accepting TCP-connection -> exiting from listener thread"));
						break;
					}
					std::auto_ptr<TcpSocket> socketAutoPtr(serverSocket.accept(listenTimeout()));
					if (shouldTerminate()) {
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener thread termination detected after accepting TCP-connection -> exiting from listener thread"));
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
					std::auto_ptr<SharedStaff> sharedStaffAutoPtr(_service.createSharedStaff(*socketAutoPtr.get()));
					socketAutoPtr.release();
					std::auto_ptr<AbstractTask> receiverTaskAutoPtr(_service.createReceiverTask(*this, *sharedStaffAutoPtr.get()));
					SharedStaff * sharedStaffPtr = sharedStaffAutoPtr.get();
					sharedStaffAutoPtr.release();
					std::auto_ptr<AbstractTask> senderTaskAutoPtr(_service.createSenderTask(*this, *sharedStaffPtr));
					sharedStaffPtr->init();
					std::list<AbstractTask *> taskList;
					taskList.push_back(receiverTaskAutoPtr.get());
					taskList.push_back(senderTaskAutoPtr.get());
					if (taskDispatcher().perform(taskList)) {
						receiverTaskAutoPtr.release();
						senderTaskAutoPtr.release();
					} else {
						warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Too many TCP-connection requests"));
					}
				}
			} catch (std::exception& e) {
				errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Asynchronous TCP-service listener execution error -> exiting from listener thread"));
			} catch (...) {
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Asynchronous TCP-service listener unknown execution error -> exiting from listener thread"));
			}
			onStop();
		}

		AbstractAsyncTcpService& _service;
	};
	//! Base class for asynchronous TCP-service receiver task
	class AbstractReceiverTask : public AbstractTask
	{
	public:
		//! Constructor
		/*!
		  \param service Reference to the asynchronous TCP-service
		  \param sharedStaff Reference to the shared staff object
		*/
		AbstractReceiverTask(AbstractAsyncTcpService& service, SharedStaff& sharedStaff) :
			AbstractTask(),
			_service(service),
			_sharedStaffPtr(&sharedStaff)
		{
			_sharedStaffPtr->_receiverTask = this;
			_sharedStaffPtr->incRef();
		}
		virtual ~AbstractReceiverTask()
		{
			if (_sharedStaffPtr->decRef() <= 0) {
				delete _sharedStaffPtr;
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "SharedStaff object has been destroyed"));
			}
		}
		//! Returns reference to the socket
		inline TcpSocket& socket() const
		{
			return _sharedStaffPtr->socket();
		}
	private:
		AbstractReceiverTask();
		AbstractReceiverTask(const AbstractReceiverTask&);						// No copy

		AbstractReceiverTask& operator=(const AbstractReceiverTask&);					// No copy

		AbstractAsyncTcpService& _service;
		SharedStaff * _sharedStaffPtr;
	};
	//! Base class for asynchronous TCP-service sender task
	class AbstractSenderTask : public AbstractTask
	{
	public:
		//! Constructor
		/*!
		  \param service Reference to the asynchronous TCP-service
		  \param sharedStaff Reference to the shared staff object
		*/
		AbstractSenderTask(AbstractAsyncTcpService& service, SharedStaff& sharedStaff) :
			AbstractTask(),
			_service(service),
			_sharedStaffPtr(&sharedStaff)
		{
			_sharedStaffPtr->_senderTask = this;
			_sharedStaffPtr->incRef();
		}
		//! Destructor
		virtual ~AbstractSenderTask()
		{
			if (_sharedStaffPtr->decRef() <= 0) {
				delete _sharedStaffPtr;
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "SharedStaff object has been destroyed"));
			}
		}
		//! Returns reference to the socket
		inline TcpSocket& socket() const
		{
			return _sharedStaffPtr->socket();
		}
	private:
		AbstractSenderTask();
		AbstractSenderTask(const AbstractSenderTask&);							// No copy

		AbstractSenderTask& operator=(const AbstractSenderTask&);					// No copy

		AbstractAsyncTcpService& _service;
		SharedStaff * _sharedStaffPtr;
	};

	//! Shared staff object creation factory method
	/*!
	  \param socket Reference to the client connection socket
	  \return Pointer to the new shared staff object
	*/
	virtual SharedStaff * createSharedStaff(TcpSocket& socket)
	{
		return new SharedStaff(socket);
	}
	//! Listener object creation factory method
	/*!
	  \param addrInfo TCP-address info to bind to
	  \param listenTimeout Timeout to wait for incoming connections
	  \param backLog Listen backlog
	  \return Pointer to new listener
	*/
	virtual AbstractListenerThread * createListener(const TcpAddrInfo& addrInfo, const Timeout& listenTimeout, unsigned int backLog)
	{
		return new ListenerThread(*this, addrInfo, listenTimeout, backLog);
	}
	//! Receiver task object creation factory method to override
	/*!
	  \param listener Reference to listener thread object
	  \param sharedStaff Reference to the shared staff object
	  \return Pointer to the new receiver task object
	*/
	virtual AbstractReceiverTask * createReceiverTask(ListenerThread& listener, SharedStaff& sharedStaff) = 0;
	//! Sender task object creation factory method to override
	/*!
	  \param listener Reference to listener thread object
	  \param sharedStaff Reference to the shared staff object
	  \return Pointer to the new sender task object
	*/
	virtual AbstractSenderTask * createSenderTask(ListenerThread& listener, SharedStaff& sharedStaff) = 0;
private:
	AbstractAsyncTcpService();
	AbstractAsyncTcpService(const AbstractAsyncTcpService&);						// No copy

	AbstractAsyncTcpService& operator=(const AbstractAsyncTcpService&);					// No copy
};

} // namespace isl

#endif

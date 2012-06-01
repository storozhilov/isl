#ifndef ISL__ABSTRACT_ASYNC_TCP_SERVICE__HXX
#define ISL__ABSTRACT_ASYNC_TCP_SERVICE__HXX

#include <isl/AbstractTcpService.hxx>


namespace isl
{

//! Base class for asynchronous TCP-service, which reads from and writes to socket in the different threads
class AbstractAsyncTcpService : public AbstractTcpService
{
public:
	//! Constructor
	/*!
	  \param owner Pointer to the owner subsystem
	  \param maxClients Maximum clients amount to serve at the same time
	  \param maxTaskQueueOverflowSize Maximum tasks queue overflow size
	*/
	AbstractAsyncTcpService(AbstractSubsystem * owner, size_t maxClients, size_t maxTaskQueueOverflowSize = 0) :
		AbstractTcpService(owner, maxClients * 2, maxTaskQueueOverflowSize)
	{}

	//! Returns maximum clients amount
	inline size_t maxClients() const
	{
		return workersAmount() / 2;
	}
	//! Sets maximum clients amount
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param newValue New maximum clients amount
	*/
	inline void setMaxClients(size_t newValue)
	{
		setWorkersAmount(newValue * 2);
	}
protected:
	class AbstractReceiverTask;
	class AbstractSenderTask;
	//! Connection, which is to be thread-safely erased during sender's and receiver's destructor invocations. Feel free to subclass.
	class Connection
	{
	public:
		Connection(TcpSocket * socketPtr) :
			_socket(socketPtr),
			_refsCount(0),
			_refsCountMutex(),
			_shouldTerminate(false),
			_shouldTerminateRwLock(),
			_receiverTask(),
			_senderTask()
		{}
		virtual ~Connection()
		{}
//		inline int incRef()
//		{
//			MutexLocker locker(_refsCountMutex);
//			return ++_refsCount;
//		}
//		inline int decRef()
//		{
//			MutexLocker locker(_refsCountMutex);
//			return --_refsCount;
//		}
//		inline void setReceiverTask(AbstractReceiverTask& receiverTask)
//		{
//			_receiverTask = &receiverTask;
//		}
		inline AbstractReceiverTask& receiverTask()
		{
			return *_receiverTask;
		}
//		inline void setSenderTask(AbstractSenderTask& senderTask)
//		{
//			_senderTask = &senderTask;
//		}
		inline AbstractSenderTask& senderTask()
		{
			return *_senderTask;
		}
		inline TcpSocket& socket()
		{
			return *_socket.get();
		}
		inline bool shouldTerminate() const
		{
			ReadLocker locker(_shouldTerminateRwLock);
			return _shouldTerminate;
		}
		inline bool setShouldTerminate(bool newValue)
		{
			WriteLocker locker(_shouldTerminateRwLock);
			bool oldValue = _shouldTerminate;
			_shouldTerminate = newValue;
			return oldValue;
		}
	private:
		Connection();
		Connection(const Connection&);							// No copy

		Connection& operator=(const Connection&);					// No copy

		inline int incRef()
		{
			MutexLocker locker(_refsCountMutex);
			return ++_refsCount;
		}
		inline int decRef()
		{
			MutexLocker locker(_refsCountMutex);
			return --_refsCount;
		}

		std::auto_ptr<TcpSocket> _socket;
		int _refsCount;
		Mutex _refsCountMutex;
		bool _shouldTerminate;
		mutable ReadWriteLock _shouldTerminateRwLock;
		AbstractReceiverTask * _receiverTask;
		AbstractSenderTask * _senderTask;

		friend class AbstractReceiverTask;
		friend class AbstractSenderTask;
	};

	//! Asynchronous TCP-service listener thread. Feel free to subclass.
	class ListenerThread : public AbstractListenerThread
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

		ListenerThread& operator=(const ListenerThread&);							// No copy

		virtual void run()
		{
			onStart();
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Listener thread has been started"));
			try {
				TcpSocket serverSocket;
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been created"));
				serverSocket.open();
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been opened"));
				serverSocket.bind(addrInfo());
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been binded"));
				serverSocket.listen(backLog());
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been switched to the listening state"));
				while (true) {
					if (shouldTerminate()) {
						Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"TCP-service termination state detected before accepting TCP-connection -> exiting from listener thread"));
						break;
					}
					std::auto_ptr<TcpSocket> socketAutoPtr(serverSocket.accept(listenTimeout()));
					if (shouldTerminate()) {
						Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"TCP-service termination state detected after accepting TCP-connection -> exiting from listener thread"));
						break;
					}
					if (!socketAutoPtr.get()) {
						// Accepting TCP-connection timeout expired
						continue;
					}
					std::wostringstream msg;
					msg << L"TCP-connection has been received from " << String::utf8Decode(socketAutoPtr.get()->remoteAddr().firstEndpoint().host) << L':' <<
						socketAutoPtr.get()->remoteAddr().firstEndpoint().port << std::endl;
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
					std::auto_ptr<Connection> connectionAutoPtr(_service.createConnection(socketAutoPtr.get()));
					socketAutoPtr.release();
					std::auto_ptr<AbstractTaskType> receiverTaskAutoPtr(_service.createReceiverTask(*this, connectionAutoPtr.get()));
					Connection * connectionPtr = connectionAutoPtr.get();
					connectionAutoPtr.release();
					std::auto_ptr<AbstractTaskType> senderTaskAutoPtr(_service.createSenderTask(*this, connectionPtr));
					std::list<AbstractTaskType *> taskList;
					taskList.push_back(receiverTaskAutoPtr.get());
					taskList.push_back(senderTaskAutoPtr.get());
					if (taskDispatcher().perform(taskList)) {
						receiverTaskAutoPtr.release();
						senderTaskAutoPtr.release();
					} else {
						Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Too many TCP-connection requests"));
					}
				}
			} catch (std::exception& e) {
				Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Asynchronous TCP-service listener execution error -> exiting from listener thread"));
			} catch (...) {
				Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Asynchronous TCP-service listener unknown execution error -> exiting from listener thread"));
			}
			onStop();
		}

		AbstractAsyncTcpService& _service;
	};
	//! Base class for asynchronous TCP-service receiver task
	class AbstractReceiverTask : public AbstractTaskType
	{
	public:
		//! Constructor
		/*!
		  \param connection Connection to use for I/O
		*/
		AbstractReceiverTask(AbstractAsyncTcpService& service, Connection * connection) :
			AbstractTaskType(),
			_service(service),
			_connection(connection)
		{
			_connection->_receiverTask = this;
			_connection->incRef();
		}
		virtual ~AbstractReceiverTask()
		{
			if (_connection->decRef() <= 0) {
				delete _connection;
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Connection object has been destroyed"));
			}
		}
		//! Returns reference to TCP-service
		inline AbstractAsyncTcpService& service()
		{
			return _service;
		}
		//! Returns reference to connection
		/*inline Connection& connection()
		{
			return _connection;
		}*/
		//! Returns reference to the socket
		inline TcpSocket& socket() const
		{
			return _connection->socket();
		}
	protected:
		// Returns true if task should be terminated
		inline bool shouldTerminate() const
		{
			return _service.shouldTerminate() || _connection->shouldTerminate();
		}
	private:
		AbstractReceiverTask();
		AbstractReceiverTask(const AbstractReceiverTask&);						// No copy

		AbstractReceiverTask& operator=(const AbstractReceiverTask&);					// No copy

		AbstractAsyncTcpService& _service;
		Connection * _connection;
	};
	//! Base class for asynchronous TCP-service sender task
	class AbstractSenderTask : public AbstractTaskType
	{
	public:
		//! Constructor
		/*!
		  \param socket Socket to use for I/O
		*/
		AbstractSenderTask(AbstractAsyncTcpService& service, Connection * connection) :
			AbstractTaskType(),
			_service(service),
			_connection(connection)
		{
			_connection->_senderTask = this;
			_connection->incRef();
		}
		virtual ~AbstractSenderTask()
		{
			if (_connection->decRef() <= 0) {
				delete _connection;
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Connection object has been destroyed"));
			}
		}
		//! Returns reference to TCP-service
		inline AbstractAsyncTcpService& service()
		{
			return _service;
		}
		//! Returns reference to connection
		/*inline Connection& connection()
		{
			return _connection;
		}*/
		//! Returns reference to the socket
		inline TcpSocket& socket() const
		{
			return _connection->socket();
		}
	protected:
		// Returns true if task should be terminated
		inline bool shouldTerminate() const
		{
			return _service.shouldTerminate() || _connection->shouldTerminate();
		}
	private:
		AbstractSenderTask();
		AbstractSenderTask(const AbstractSenderTask&);							// No copy

		AbstractSenderTask& operator=(const AbstractSenderTask&);					// No copy

		AbstractAsyncTcpService& _service;
		Connection * _connection;
	};

	//! Creating connection factory method
	/*!
	  \param socket TCP-socket for collaborative usage
	  \return std::auto_ptr with new connection
	*/
	virtual std::auto_ptr<Connection> createConnection(TcpSocket * socket)
	{
		return std::auto_ptr<Connection>(new Connection(socket));
	}
	//! Creating listener factory method
	/*!
	  \param addrInfo TCP-address info to bind to
	  \param listenTimeout Timeout to wait for incoming connections
	  \param backLog Listen backlog
	  \return New listener's auto_ptr
	*/
	virtual std::auto_ptr<AbstractListenerThread> createListener(const TcpAddrInfo& addrInfo, const Timeout& listenTimeout, unsigned int backLog)
	{
		return std::auto_ptr<AbstractListenerThread>(new ListenerThread(*this, addrInfo, listenTimeout, backLog));
	}
	//! Creating receiver task factory method to override
	/*!
	  \param socket TCP-socket for I/O
	  \return std::auto_ptr with pointer to the new receiver task
	*/
	virtual std::auto_ptr<AbstractReceiverTask> createReceiverTask(ListenerThread& listener, Connection * connectionPtr) = 0;
	//! Creating sender task factory method to override
	/*!
	  \param socket TCP-socket for I/O
	  \return std::auto_ptr with pointer to the new sender task
	*/
	virtual std::auto_ptr<AbstractSenderTask> createSenderTask(ListenerThread& listener, Connection * connectionPtr) = 0;
private:
	AbstractAsyncTcpService();
	AbstractAsyncTcpService(const AbstractAsyncTcpService&);						// No copy

	AbstractAsyncTcpService& operator=(const AbstractAsyncTcpService&);					// No copy
};

} // namespace isl

#endif

#ifndef ISL__ABSTRACT_ASYNC_TCP_SERVICE__HXX
#define ISL__ABSTRACT_ASYNC_TCP_SERVICE__HXX

#include <isl/AbstractTcpService.hxx>


namespace isl
{

//! Base class for the TCP-service implementation
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
	//! Shared socket for proper thread-safe socket disposal during receiver and sender task destruction
	class SharedSocket
	{
	public:
		SharedSocket(TcpSocket * socketPtr) :
			_socket(socketPtr),
			_refsCount(0),
			_refsCountMutex()
		{}
		int incRef()
		{
			MutexLocker locker(_refsCountMutex);
			return ++_refsCount;
		}
		int decRef()
		{
			MutexLocker locker(_refsCountMutex);
			return --_refsCount;
		}
		TcpSocket& socket()
		{
			return *_socket.get();
		}
	private:
		std::auto_ptr<TcpSocket> _socket;
		int _refsCount;
		Mutex _refsCountMutex;
	};

	//! Asynchronous TCP-service listener thread
	class Listener : public AbstractListener
	{
	public:
		//! Constructor
		/*!
		  \param service Reference to TCP-service object
		  \param port TCP-port to bind to
		  \param interfaces Network intarfaces to bind to
		  \param backLog Listen backlog
		*/
		Listener(AbstractAsyncTcpService& service, unsigned int port, const Timeout& listenTimeout, const std::list<std::string>& interfaces, unsigned int backLog) :
			AbstractListener(service, port, listenTimeout, interfaces, backLog),
			_service(service)
		{}
	private:
		Listener();
		Listener(const Listener&);								// No copy

		Listener& operator=(const Listener&);							// No copy

		virtual void run()
		{
			onStart();
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Listener has been started"));
			try {
				TcpSocket serverSocket;
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been created"));
				serverSocket.open();
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been opened"));
				serverSocket.bind(port(), interfaces());
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
					std::auto_ptr<SharedSocket> sharedSocketAutoPtr(new SharedSocket(socketAutoPtr.get()));
					socketAutoPtr.release();
					std::auto_ptr<TaskDispatcher::AbstractTask> receiverTaskAutoPtr(_service.createReceiverTask(sharedSocketAutoPtr.get()));
					SharedSocket * sharedSocketPtr = sharedSocketAutoPtr.get();
					sharedSocketAutoPtr.release();
					std::auto_ptr<TaskDispatcher::AbstractTask> senderTaskAutoPtr(_service.createSenderTask(sharedSocketPtr));
					std::list<TaskDispatcher::AbstractTask *> taskList;
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
	class AbstractReceiverTask : public TaskDispatcher::AbstractTask
	{
	public:
		//! Constructor
		/*!
		  \param sharedSocket Shared socket to use for I/O
		*/
		AbstractReceiverTask(AbstractAsyncTcpService& service, SharedSocket * sharedSocket) :
			TaskDispatcher::AbstractTask(),
			_service(service),
			_sharedSocket(sharedSocket)
		{
			_sharedSocket->incRef();
		}
		virtual ~AbstractReceiverTask()
		{
			if (_sharedSocket->decRef() <= 0) {
				delete _sharedSocket;
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Client connection socket has been destroyed"));
			}
		}
		//! Returns reference to TCP-service
		inline AbstractAsyncTcpService& service()
		{
			return _service;
		}
		//! Returns reference to the socket
		inline TcpSocket& socket() const
		{
			return _sharedSocket->socket();
		}
	protected:
		// Returns true if task should be terminated
		inline bool shouldTerminate() const
		{
			AbstractSubsystem::State state = _service.state();
			return state != AbstractSubsystem::StartingState && state != AbstractSubsystem::RunningState;
		}
	private:
		AbstractReceiverTask();
		AbstractReceiverTask(const AbstractReceiverTask&);						// No copy

		AbstractReceiverTask& operator=(const AbstractReceiverTask&);					// No copy

		AbstractAsyncTcpService& _service;
		SharedSocket * _sharedSocket;
	};
	//! Base class for asynchronous TCP-service sender task
	class AbstractSenderTask : public TaskDispatcher::AbstractTask
	{
	public:
		//! Constructor
		/*!
		  \param socket Socket to use for I/O
		*/
		AbstractSenderTask(AbstractAsyncTcpService& service, SharedSocket * sharedSocket) :
			TaskDispatcher::AbstractTask(),
			_service(service),
			_sharedSocket(sharedSocket)
		{
			_sharedSocket->incRef();
		}
		virtual ~AbstractSenderTask()
		{
			if (_sharedSocket->decRef() <= 0) {
				delete _sharedSocket;
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Client connection socket has been destroyed"));
			}
		}
		//! Returns reference to TCP-service
		inline AbstractAsyncTcpService& service()
		{
			return _service;
		}
		//! Returns reference to the socket
		inline TcpSocket& socket() const
		{
			return _sharedSocket->socket();
		}
	protected:
		// Returns true if task should be terminated
		inline bool shouldTerminate() const
		{
			AbstractSubsystem::State state = _service.state();
			return state != AbstractSubsystem::StartingState && state != AbstractSubsystem::RunningState;
		}
	private:
		AbstractSenderTask();
		AbstractSenderTask(const AbstractSenderTask&);							// No copy

		AbstractSenderTask& operator=(const AbstractSenderTask&);					// No copy

		AbstractAsyncTcpService& _service;
		SharedSocket * _sharedSocket;
	};

	//! Creating listener factory method
	/*!
	  \param port TCP-port to bind to
	  \param listenTimeout Timeout to wait for incoming connections
	  \param interfaces Network interfaces to bind to
	  \param backLog Listen backlog
	  \return New listener
	*/
	virtual AbstractListener * createListener(unsigned int port, const Timeout& listenTimeout, const std::list<std::string>& interfaces, unsigned int backLog)
	{
		return new Listener(*this, port, listenTimeout, interfaces, backLog);
	}
	//! Creating receiver task factory method to override
	/*!
	  \param socket TCP-socket for I/O
	*/
	//virtual AbstractReceiverTask * createReceiverTask(TcpSocket * socket) = 0;
	virtual AbstractReceiverTask * createReceiverTask(SharedSocket * sharedSocketPtr) = 0;
	//! Creating sender task factory method to override
	/*!
	  \param socket TCP-socket for I/O
	*/
	//virtual AbstractSenderTask * createSenderTask(TcpSocket * socket) = 0;
	virtual AbstractSenderTask * createSenderTask(SharedSocket * sharedSocketPtr) = 0;
private:
	AbstractAsyncTcpService();
	AbstractAsyncTcpService(const AbstractAsyncTcpService&);						// No copy

	AbstractAsyncTcpService& operator=(const AbstractAsyncTcpService&);					// No copy

	virtual void beforeStart()
	{
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Starting asynchronous TCP-service"));
		AbstractTcpService::beforeStart();
	}
	virtual void afterStart()
	{
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Asynchronous TCP-service has been started"));
	}
	virtual void beforeStop()
	{
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Stopping asynchronous TCP-service"));
	}
	virtual void afterStop()
	{
		AbstractTcpService::afterStop();
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Asynchronous TCP-service has been stopped"));
	}
};

} // namespace isl

#endif


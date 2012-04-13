#ifndef ISL__ABSTRACT_SYNC_TCP_SERVICE__HXX
#define ISL__ABSTRACT_SYNC_TCP_SERVICE__HXX

#include <isl/AbstractTcpService.hxx>

namespace isl
{

//! Base class for synchronous TCP-service, which reads from and writes to socket in the same thread
class AbstractSyncTcpService : public AbstractTcpService
{
public:
	//! Constructor
	/*!
	  \param owner Pointer to the owner subsystem
	  \param maxClients Maximum clients amount to serve at the same time
	  \param maxTaskQueueOverflowSize Maximum tasks queue overflow size
	*/
	AbstractSyncTcpService(AbstractSubsystem * owner, size_t maxClients, size_t maxTaskQueueOverflowSize = 0) :
		AbstractTcpService(owner, maxClients, maxTaskQueueOverflowSize)
	{}

	//! Returns maximum clients amount
	inline size_t maxClients() const
	{
		return workersAmount();
	}
	//! Sets maximum clients amount
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param newValue New maximum clients amount
	*/
	inline void setMaxClients(size_t newValue)
	{
		setWorkersAmount(newValue);
	}
protected:
	//! Synchronous TCP-service listener thread. Feel free to subclass.
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
		ListenerThread(AbstractSyncTcpService& service, const TcpAddrInfo& addrInfo, const Timeout& listenTimeout, unsigned int backLog) :
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
					std::auto_ptr<AbstractTaskType> taskAutoPtr(_service.createTask(socketAutoPtr.get(), *this));
					socketAutoPtr.release();
					if (taskDispatcher().perform(taskAutoPtr.get())) {
						taskAutoPtr.release();
					} else {
						Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Too many TCP-connection requests"));
					}
				}
			} catch (std::exception& e) {
				Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Synchronous TCP-service listener execution error -> exiting from listener thread"));
			} catch (...) {
				Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Synchronous TCP-service listener unknown execution error -> exiting from listener thread"));
			}
			onStop();
		}

		AbstractSyncTcpService& _service;
	};

	//! Base class for synchronous TCP-service task
	class AbstractTask : public AbstractTaskType
	{
	public:
		//! Constructor
		/*!
		  \param socket Socket to use for I/O
		*/
		AbstractTask(AbstractSyncTcpService& service, TcpSocket * socket) :
			AbstractTaskType(),
			_service(service),
			_socket(socket)
		{}
		//! Returns reference to TCP-service
		inline AbstractSyncTcpService& service()
		{
			return _service;
		}
		//! Returns reference to the socket
		inline TcpSocket& socket() const
		{
			return *_socket.get();
		}
	protected:
		// Returns true if task should be terminated
		inline bool shouldTerminate() const
		{
			AbstractSubsystem::State state = _service.state();
			return state != AbstractSubsystem::StartingState && state != AbstractSubsystem::RunningState;
		}
	private:
		AbstractTask();
		AbstractTask(const AbstractTask&);								// No copy

		AbstractTask& operator=(const AbstractTask&);							// No copy

		AbstractSyncTcpService& _service;
		std::auto_ptr<TcpSocket> _socket;
	};

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
	//! Creating task factory method to override
	/*!
	  \param socket TCP-socket for I/O
	  \return auto_ptr to new task
	*/
	virtual std::auto_ptr<AbstractTask> createTask(TcpSocket * socket, ListenerThread& listener) = 0;
private:
	AbstractSyncTcpService();
	AbstractSyncTcpService(const AbstractSyncTcpService&);						// No copy

	AbstractSyncTcpService& operator=(const AbstractSyncTcpService&);				// No copy

	virtual void beforeStart()
	{
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Starting synchronous TCP-service"));
		AbstractTcpService::beforeStart();
	}
	virtual void afterStart()
	{
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Synchronous TCP-service has been started"));
	}
	virtual void beforeStop()
	{
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Stopping synchronous TCP-service"));
	}
	virtual void afterStop()
	{
		AbstractTcpService::afterStop();
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Synchronous TCP-service has been stopped"));
	}
};

} // namespace isl

#endif

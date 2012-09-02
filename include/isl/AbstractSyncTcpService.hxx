#ifndef ISL__ABSTRACT_SYNC_TCP_SERVICE__HXX
#define ISL__ABSTRACT_SYNC_TCP_SERVICE__HXX

#include <isl/common.hxx>
#include <isl/AbstractTcpService.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>

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
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener thread termination state detected before accepting TCP-connection -> exiting from listener thread"));
						break;
					}
					std::auto_ptr<TcpSocket> socketAutoPtr(serverSocket.accept(listenTimeout()));
					if (shouldTerminate()) {
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Listener thread termination state detected after accepting TCP-connection -> exiting from listener thread"));
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
					std::auto_ptr<AbstractTaskType> taskAutoPtr(_service.createTask(*this, *socketAutoPtr.get()));
					socketAutoPtr.release();
					if (taskDispatcher().perform(taskAutoPtr.get())) {
						taskAutoPtr.release();
					} else {
						warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Too many TCP-connection requests"));
					}
				}
			} catch (std::exception& e) {
				errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Synchronous TCP-service listener execution error -> exiting from listener thread"));
			} catch (...) {
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Synchronous TCP-service listener unknown execution error -> exiting from listener thread"));
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
		  \param socket Reference to the client connection socket
		*/
		AbstractTask(AbstractSyncTcpService& service, TcpSocket& socket) :
			AbstractTaskType(),
			_service(service),
			_socketAutoPtr(&socket)
		{}
		//! Returns reference to TCP-service
		inline AbstractSyncTcpService& service()
		{
			return _service;
		}
		//! Returns reference to the socket
		inline TcpSocket& socket() const
		{
			return *_socketAutoPtr.get();
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
		std::auto_ptr<TcpSocket> _socketAutoPtr;
	};

	//! Creating listener factory method
	/*!
	  \param addrInfo TCP-address info to bind to
	  \param listenTimeout Timeout to wait for incoming connections
	  \param backLog Listen backlog
	  \return Auto-pointer to the new listener object
	*/
	virtual std::auto_ptr<AbstractListenerThread> createListener(const TcpAddrInfo& addrInfo, const Timeout& listenTimeout, unsigned int backLog)
	{
		return std::auto_ptr<AbstractListenerThread>(new ListenerThread(*this, addrInfo, listenTimeout, backLog));
	}
	//! Creating task factory method to override
	/*!
	  \param listener Reference to listener thread object
	  \param socket Reference to the client connection socket
	  \return Auto-pointer to the new task object
	*/
	virtual std::auto_ptr<AbstractTask> createTask(ListenerThread& listener, TcpSocket& socket) = 0;
private:
	AbstractSyncTcpService();
	AbstractSyncTcpService(const AbstractSyncTcpService&);						// No copy

	AbstractSyncTcpService& operator=(const AbstractSyncTcpService&);				// No copy

	virtual void beforeStart()
	{
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Starting synchronous TCP-service"));
		AbstractTcpService::beforeStart();
	}
	virtual void afterStart()
	{
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Synchronous TCP-service has been started"));
	}
	virtual void beforeStop()
	{
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Stopping synchronous TCP-service"));
	}
	virtual void afterStop()
	{
		AbstractTcpService::afterStop();
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Synchronous TCP-service has been stopped"));
	}
};

} // namespace isl

#endif

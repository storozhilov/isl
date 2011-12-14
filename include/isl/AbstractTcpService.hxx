#ifndef ISL__ABSTRACT_TCP_SERVICE__HXX
#define ISL__ABSTRACT_TCP_SERVICE__HXX

#include <isl/AbstractSubsystem.hxx>
#include <isl/AbstractTask.hxx>
#include <isl/Timeout.hxx>
#include <isl/TaskDispatcher.hxx>
#include <memory>

namespace isl
{

class TcpSocket;

//! Base class for the TCP-service implementation
/*
   TODO Documentation!!!
   TODO Interfaces support
*/
class AbstractTcpService : public AbstractSubsystem
{
public:
	//! Constructor
	/*!
	  \param owner Pointer to the owner subsystem.
	  \param port TCP-port to listen to (TODO Multiple ports support).
	  \param maxClients Maximum clients amount to serve at the same time.
	  \param timeout Accepting connection timeout.
	  \param interfaces Interfaces to bind to. Default is empty list which means to bind to all interfaces.
	  \param backLog Listen backlog parameter. Default to 15.
	  \param maxTaskQueueOverflowSize Maximum tasks queue overflow size. Default to 0.
	*/
	AbstractTcpService(AbstractSubsystem * owner, unsigned int port, unsigned int maxClients,
			const Timeout& timeout = Timeout(1), const std::list<std::wstring>& interfaces = std::list<std::wstring>(),
			unsigned int backLog = 15, unsigned int maxTaskQueueOverflowSize = 0);

	//! Base class for TCP-service task
	class AbstractTask : public ::isl::AbstractTask
	{
	public:
		//! Constructor
		/*!
		  \param socket Socket to use for I/O
		*/
		AbstractTask(TcpSocket * socket) :
			::isl::AbstractTask(),
			_socket(socket)
		{}
		//! Returns reference to the socket
		inline TcpSocket& socket() const
		{
			return *_socket;
		}
	private:
		AbstractTask();
		AbstractTask(const AbstractTask&);							// No copy

		AbstractTask& operator=(const AbstractTask&);						// No copy

		std::auto_ptr<TcpSocket> _socket;
	};

	//! Thread-safely returns listening TCP-port
	inline unsigned int port() const
	{
		ReadLocker locker(_portRwLock);
		return _port;
	}
	//! Thread-safely sets listening TCP-port
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param newValue New TCP-port listen to
	*/
	inline void setPort(unsigned int newValue)
	{
		WriteLocker locker(_portRwLock);
		_port = newValue;
	}
	//! Thread-safely returns accepting connection timeout
	inline Timeout timeout() const
	{
		ReadLocker locker(_timeoutRwLock);
		return _timeout;
	}
	//! Thread-safely sets new accepting connection timeout
	/*!
	  Changes will take place on the next performing accepting connection operation
	  \param newValue New accepting connection timeout
	*/
	inline void setTimeout(const Timeout& newValue)
	{
		WriteLocker locker(_timeoutRwLock);
		_timeout = newValue;
	}
	//! Thread-safely returns maximum clients amount
	inline unsigned int maxClients() const
	{
		return _taskDispatcher.workersCount();
	}
	//! Thread-safely sets new maximum clients amount
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param newValue New maximum clients amount
	*/
	inline void setMaxClients(unsigned int newValue)
	{
		_taskDispatcher.setWorkersCount(newValue);
	}
	//! Thread-safely returns interfaces that should be listen to
	inline std::list<std::wstring> interfaces() const
	{
		ReadLocker locker(_interfacesRwLock);
		return _interfaces;
	}
	//! Thread-safely sets new interfaces that should be listen to
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param newValue New interfaces that should be listen to
	*/
	inline void setInterfaces(const std::list<std::wstring>& newValue)
	{
		WriteLocker locker(_interfacesRwLock);
		_interfaces = newValue;
	}
	//! Thread-safely returns listen backlog
	inline unsigned int backLog() const
	{
		ReadLocker locker(_backLogRwLock);
		return _backLog;
	}
	//! Thread-safely sets new listen backlog
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param newValue New listen backlog
	*/
	inline void setBackLog(unsigned int newValue)
	{
		WriteLocker locker(_backLogRwLock);
		_backLog = newValue;
	}
	//! Thread-safely returns maximum task queue overflow size.
	inline unsigned int maxTaskQueueOverflowSize() const
	{
		return _taskDispatcher.maxTaskQueueOverflowSize();
	}
	//! Thread-safely sets the new maximum task queue overflow size.
	/*!
	  Changes will take place on the next task performing operation
	  \param newValue New maximum task queue overflow size
	*/
	inline void setMaxTaskQueueOverflowSize(unsigned int newValue)
	{
		_taskDispatcher.setMaxTaskQueueOverflowSize(newValue);
	}
	//! Asynchronously starts subsystem
	virtual void start();
	//! Synchronously stops subsystem
	virtual void stop();
protected:
	//! Creating task factory method to override
	/*!
	  \param socket TCP-socket for I/O
	*/
	virtual AbstractTask * createTask(TcpSocket * socket) = 0;
private:
	class ListenerThread : public Thread
	{
	public:
		ListenerThread(AbstractTcpService& service);
	private:
		ListenerThread();
		ListenerThread(const ListenerThread&);							// No copy

		ListenerThread& operator=(const ListenerThread&);					// No copy

		virtual void run();

		inline void sleep()
		{
			MutexLocker locker(_sleepCond.mutex());
			_sleepCond.wait(_service.timeout());
		}

		AbstractTcpService& _service;
		WaitCondition _sleepCond;
	};

	AbstractTcpService();
	AbstractTcpService(const AbstractTcpService&);							// No copy

	AbstractTcpService& operator=(const AbstractTcpService&);					// No copy

	TaskDispatcher _taskDispatcher;
	ListenerThread _listenerThread;
	unsigned int _port;
	mutable ReadWriteLock _portRwLock;
	Timeout _timeout;
	mutable ReadWriteLock _timeoutRwLock;
	std::list<std::wstring> _interfaces;
	mutable ReadWriteLock _interfacesRwLock;
	unsigned int _backLog;
	mutable ReadWriteLock _backLogRwLock;
};

} // namespace isl

#endif


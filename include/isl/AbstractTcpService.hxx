#ifndef ISL__ABSTRACT_TCP_SERVICE__HXX
#define ISL__ABSTRACT_TCP_SERVICE__HXX

#include <isl/AbstractSubsystem.hxx>
#include <isl/Timeout.hxx>
#include <isl/TaskDispatcher.hxx>
#include <memory>

namespace isl
{

class TcpSocket;

//! Base class for TCP-service
class AbstractTcpService : public AbstractSubsystem
{
public:
	//! Constructor
	/*!
	  \param owner Pointer to the owner subsystem.
	  \param port TCP-port to listen to (TODO Multiple ports support).
	  \param maxClients Maximum clients amount to serve at the same time.
	  \param timeout I/O timeout.
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

	//! Returns listening TCP-port
	inline unsigned int port() const
	{
		return _port;
	}
	//! Returns I/O timeout
	inline Timeout timeout() const
	{
		return _timeout;
	}
	//! Returns maximum clients amount
	inline unsigned int maxClients() const
	{
		return _taskDispatcher.workersCount();
	}
	//! Returns binded interfaces
	inline std::list<std::wstring> interfaces() const
	{
		return _interfaces;
	}
	//! Returns listen backlog
	inline unsigned int backLog() const
	{
		return _backLog;
	}
	//! Returns maximum tasks queue overflow size
	inline unsigned int maxTaskQueueOverflowSize() const
	{
		return _taskDispatcher.maxTaskQueueOverflowSize();
	}
	//! Asynchronously starts subsystem
	virtual bool start();
	//! Synchronously stops subsystem
	virtual void stop();
	//! Restarts subsystem
	virtual bool restart();
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
	Timeout _timeout;
	std::list<std::wstring> _interfaces;
	unsigned int _backLog;
};

} // namespace isl

#endif


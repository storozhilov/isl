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
class AbstractTcpService : public AbstractSubsystem
{
public:
	//! Constructor
	/*!
	  \param owner Pointer to the owner subsystem.
	  \param maxClients Maximum clients amount to serve at the same time.
	  \param maxTaskQueueOverflowSize Maximum tasks queue overflow size. Default to 0.
	*/
	AbstractTcpService(AbstractSubsystem * owner, size_t maxClients, size_t maxTaskQueueOverflowSize = 0);
	//! Desctructor
	virtual ~AbstractTcpService();

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

	//! Adds listener to the service
	/*!
	  \param port TCP-port to listen to
	  \param timeout Timeout to wait for incoming connections
	  \param interfaces Network intarfaces to bind to
	  \param backLog Listen backlog
	  \return Listener id
	*/
	unsigned int addListener(unsigned int port, const Timeout& timeout = Timeout::defaultTimeout(), const std::list<std::string>& interfaces = std::list<std::string>(), unsigned int backLog = 15);
	//! Updates listener
	/*!
	  \param id Listener id
	  \param port TCP-port to listen to
	  \param timeout Timeout to wait for incoming connections
	  \param interfaces Network intarfaces to bind to
	  \param backLog Listen backlog
	*/
	void updateListener(unsigned int id, unsigned int port, const Timeout& timeout = Timeout::defaultTimeout(), const std::list<std::string>& interfaces = std::list<std::string>(), unsigned int backLog = 15);
	//! Removes listener
	/*!
	  \param id Id of the listener to remove
	*/
	void removeListener(unsigned int id);
	//! Resets all listeners
	void resetListeners();
	//! Thread-safely returns maximum clients amount
	inline size_t maxClients() const
	{
		return _taskDispatcher.workersCount();
	}
	//! Thread-safely sets new maximum clients amount
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param newValue New maximum clients amount
	*/
	inline void setMaxClients(size_t newValue)
	{
		_taskDispatcher.setWorkersCount(newValue);
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
	class Listener;

	class ListenerThread : public Thread
	{
	public:
		ListenerThread(Listener * listener);
	private:
		ListenerThread();
		ListenerThread(const ListenerThread&);							// No copy

		ListenerThread& operator=(const ListenerThread&);					// No copy

		virtual void run();

		inline void sleep()
		{
			MutexLocker locker(_sleepCond.mutex());
			_sleepCond.wait(_listener->_timeout);
		}

		Listener * _listener;
		WaitCondition _sleepCond;
	};

	class Listener : public AbstractSubsystem
	{
	public:
		Listener(AbstractTcpService& _service, unsigned int port, const Timeout& timeout, const std::list<std::string>& interfaces, unsigned int backLog);

		virtual void start();
		virtual void stop();
	private:
		AbstractTcpService& _service;
		unsigned int _port;
		Timeout _timeout;
		std::list<std::string> _interfaces;
		unsigned int _backLog;
		ListenerThread _listenerThread;

		friend class ListenerThread;
		friend class AbstractTcpService;
	};

	typedef std::map<unsigned int, Listener *> Listeners;

	AbstractTcpService();
	AbstractTcpService(const AbstractTcpService&);							// No copy

	AbstractTcpService& operator=(const AbstractTcpService&);					// No copy

	inline void resetListenersUnsafe()
	{
		for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
			delete i->second;
		}
	}

	Mutex _startStopMutex;
	unsigned int _lastListenerId;
	Listeners _listeners;
	TaskDispatcher _taskDispatcher;
};

} // namespace isl

#endif


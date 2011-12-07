#ifndef ISL__ABSTRACT_TCP_SERVICE__HXX
#define ISL__ABSTRACT_TCP_SERVICE__HXX

#include <isl/AbstractSubsystem.hxx>
#include <isl/Timeout.hxx>
#include <isl/TaskDispatcher.hxx>
#include <isl/AbstractTcpTask.hxx>

namespace isl
{

class TcpSocket;

class AbstractTcpService : public AbstractSubsystem
{
public:
	AbstractTcpService(AbstractSubsystem * owner, unsigned int port, unsigned int maxClients,
			const Timeout& timeout = Timeout(1), const std::list<std::wstring>& interfaces = std::list<std::wstring>(),
			unsigned int backLog = 15, unsigned int maxTaskQueueOverflowSize = 0);

	inline unsigned int port() const
	{
		return _port;
	}
	inline Timeout timeout() const
	{
		return _timeout;
	}
	inline unsigned int maxClients() const
	{
		return _taskDispatcher.workersCount();
	}
	inline std::list<std::wstring> interfaces() const
	{
		return _interfaces;
	}
	inline unsigned int backLog() const
	{
		return _backLog;
	}
	inline unsigned int maxTaskQueueOverflowSize() const
	{
		return _taskDispatcher.maxTaskQueueOverflowSize();
	}

	virtual bool start();
	virtual void stop();
	virtual bool restart();
protected:
	virtual AbstractTcpTask * createTask(TcpSocket * socket) = 0;
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
	Timeout _timeout;
	unsigned int _port;
	std::list<std::wstring> _interfaces;
	unsigned int _backLog;
};

} // namespace isl

#endif


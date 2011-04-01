#ifndef ISL__ABSTRACT_TCP_LISTENER__HXX
#define ISL__ABSTRACT_TCP_LISTENER__HXX

#include <isl/AbstractSubsystem.hxx>
#include <isl/Timeout.hxx>
#include <isl/TaskDispatcher.hxx>
#include <isl/AbstractTcpTask.hxx>

namespace isl
{

class TcpSocket;

class AbstractTcpListener : public AbstractSubsystem
{
public:
	AbstractTcpListener(AbstractSubsystem * owner, TaskDispatcher& taskDispatcher, unsigned int port,
			const Timeout& timeout = Timeout(1), const std::list<std::wstring>& interfaces = std::list<std::wstring>());

	Timeout timeout() const;
	void setTimeout(const Timeout& newTimeout);
	unsigned int port() const;
	void setPort(unsigned int newPort);
	std::list<std::wstring> interfaces() const;
	void setInterfaces(const std::list<std::wstring>& newInterfaces = std::list<std::wstring>());
	unsigned int backLog() const;
	void setBackLog(unsigned int newBackLog);
protected:
	virtual AbstractTcpTask * createTask(TcpSocket * socket) = 0;
private:
	class ListenerThread : public Thread
	{
	public:
		ListenerThread(AbstractTcpListener& listener);
	private:
		ListenerThread();
		ListenerThread(const ListenerThread&);							// No copy

		ListenerThread& operator=(const ListenerThread&);					// No copy

		virtual void run();

		void sleep();
		bool keepRunning();

		AbstractTcpListener& _listener;
		WaitCondition _sleepCond;
	};

	AbstractTcpListener();
	AbstractTcpListener(const AbstractTcpListener&);						// No copy

	AbstractTcpListener& operator=(const AbstractTcpListener&);					// No copy

	virtual void onStartCommand();
	virtual void onStopCommand();

	TaskDispatcher& _taskDispatcher;
	Timeout _timeout;
	mutable ReadWriteLock _timeoutRWLock;
	unsigned int _port;
	mutable ReadWriteLock _portRWLock;
	std::list<std::wstring> _interfaces;
	mutable ReadWriteLock _interfacesRWLock;
	unsigned int _backLog;
	mutable ReadWriteLock _backLogRWLock;
	ListenerThread _listenerThread;
};

} // namespace isl

#endif


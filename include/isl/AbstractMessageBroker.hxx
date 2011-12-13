#ifndef ISL__ABSTRACT_MESSAGE_BROKER__HXX
#define ISL__ABSTRACT_MESSAGE_BROKER__HXX

#include <isl/AbstractSubsystem.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/Thread.hxx>
#include <isl/Timeout.hxx>
#include <isl/TaskDispatcher.hxx>
#include <isl/DateTime.hxx>
#include <deque>
#include <memory>

namespace isl
{

class AbstractMessageBroker : public AbstractSubsystem
{
private:
	class SenderTaskTerminator;
public:
	AbstractMessageBroker(AbstractSubsystem * owner, unsigned int port, unsigned int maxClients,
			unsigned int sendQueueSize = 100, const Timeout& timeout = Timeout(1),
			const std::list<std::wstring>& interfaces = std::list<std::wstring>(),
			unsigned int backLog = 15);

	class AbstractMessage
	{
	public:
		// TODO ???
		//AbstractMessage() :
		//	_constructed(DateTime::now())
		//{}
		virtual ~AbstractMessage()
		{}

		//inline DateTime constructed() const
		//{
		//	return _constructed;
		//}

		virtual AbstractMessage * clone() const = 0;
	private:
		//DateTime _constructed;
	};

	class ReceiverTask;

	class SenderTask : public AbstractTask
	{
	public:
		SenderTask(AbstractMessageBroker& broker, TcpSocket * socket);
		virtual ~SenderTask();

		inline bool shouldTerminate() const
		{
			MutexLocker locker(_sendCond.mutex());
			return _terminateFlag;
		}
		bool sendMessage(AbstractMessage * msg);
	private:
		SenderTask();
		SenderTask(const SenderTask&);								// No copy

		SenderTask& operator=(const SenderTask&);						// No copy

		inline void terminate()
		{
			MutexLocker locker(_sendCond.mutex());
			_terminateFlag = true;
			_sendCond.wakeOne();
		}

		typedef std::deque<AbstractMessage *> MessageQueue;

		virtual void executeImplementation(Worker& worker);

		AbstractMessageBroker& _broker;
		std::auto_ptr<TcpSocket> _socketAutoPtr;
		mutable WaitCondition _sendCond;
		bool _terminateFlag;
		MessageQueue _messageQueue;

		friend class SenderTaskTerminator;
		friend class ReceiverTask;
	};


	class ReceiverTask : public AbstractTask
	{
	public:
		ReceiverTask(SenderTask& senderTask);

		inline SenderTask& senderTask() const
		{
			return _senderTask;
		}
		inline void terminate()
		{
			WriteLocker locker(_terminateFlagRWLock);
			_terminateFlag = true;
		}
		inline bool shouldTerminate() const
		{
			ReadLocker locker(_terminateFlagRWLock);
			return _terminateFlag;
		}
	private:
		ReceiverTask();
		ReceiverTask(const ReceiverTask&);							// No copy

		ReceiverTask& operator=(const ReceiverTask&);						// No copy

		virtual void executeImplementation(Worker& worker);

		AbstractMessageBroker& _broker;
		TcpSocket& _socket;
		SenderTask& _senderTask;
		bool _terminateFlag;
		mutable ReadWriteLock _terminateFlagRWLock;
	};

	inline unsigned int port() const
	{
		return _port;
	}
	inline unsigned int sendQueueSize() const
	{
		return _sendQueueSize;
	}
	inline Timeout timeout() const
	{
		return _timeout;
	}
	inline unsigned int maxClients() const
	{
		return _taskDispatcher.workersCount() / 2;
	}
	inline std::list<std::wstring> interfaces() const
	{
		return _interfaces;
	}
	inline unsigned int backLog() const
	{
		return _backLog;
	}
	inline bool isRunning() const
	{
		AbstractSubsystem::State brokerState = state();
		return (brokerState == StartingState) || (brokerState == RunningState);
	}

	virtual bool start();
	virtual void stop();
	virtual bool restart();
protected:
	virtual SenderTask * createSenderTask(TcpSocket * socket);
	virtual ReceiverTask * createReceiverTask(SenderTask& senderTask);

	virtual AbstractMessage * recieveMessage(TcpSocket& socket, ReceiverTask& recieverTask) = 0;
	virtual void processMessage(const AbstractMessage& message, ReceiverTask& recieverTask, SenderTask& senderTask) = 0;
	virtual void sendMessage(TcpSocket& socket, const AbstractMessage& message, SenderTask& senderTask) = 0;
private:
	//! Message broker's TCP-listener
	class ListenerThread : public Thread
	{
	public:
		ListenerThread(AbstractMessageBroker& broker);
	private:
		ListenerThread();
		ListenerThread(const ListenerThread&);							// No copy

		ListenerThread& operator=(const ListenerThread&);					// No copy

		virtual void run();

		void sleep()
		{
			MutexLocker locker(_sleepCond.mutex());
			_sleepCond.wait(_broker._timeout);
		}

		AbstractMessageBroker& _broker;
		WaitCondition _sleepCond;
	};
	//! Helper class - the object terminates sender task in it's destructor
	class SenderTaskTerminator
	{
	public:
		SenderTaskTerminator(SenderTask& senderTask) :
			_senderTask(senderTask)
		{}
		~SenderTaskTerminator()
		{
			_senderTask.terminate();
		}
	private:
		SenderTaskTerminator();
		SenderTaskTerminator(const SenderTaskTerminator&);					// No copy

		SenderTaskTerminator& operator=(const SenderTaskTerminator&);				// No copy

		SenderTask& _senderTask;
	};

	AbstractMessageBroker();
	AbstractMessageBroker(const AbstractMessageBroker&);						// No copy

	AbstractMessageBroker& operator=(const AbstractMessageBroker&);					// No copy

	TaskDispatcher _taskDispatcher;
	ListenerThread _listenerThread;
	unsigned int _port;
	unsigned int _sendQueueSize;
	Timeout _timeout;
	std::list<std::wstring> _interfaces;
	unsigned int _backLog;
};

} // namespace isl

#endif


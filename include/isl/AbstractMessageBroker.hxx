#ifndef ISL__ABSTRACT_MESSAGE_BROKER__HXX
#define ISL__ABSTRACT_MESSAGE_BROKER__HXX

#include <isl/AbstractSubsystem.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/Thread.hxx>
#include <isl/Timeout.hxx>
#include <isl/TaskDispatcher.hxx>
#include <isl/AbstractTask.hxx>
#include <isl/DateTime.hxx>
#include <deque>
#include <memory>

namespace isl
{

//! Base class for the TCP message broker implementation
/*
   TODO Documentation!!!
*/
class AbstractMessageBroker : public AbstractSubsystem
{
private:
	class SenderTaskTerminator;
public:
	AbstractMessageBroker(AbstractSubsystem * owner, unsigned int port, size_t maxClients,
			size_t sendQueueSize = 100, const Timeout& timeout = Timeout(1),
			const std::list<std::string>& interfaces = std::list<std::string>(),
			unsigned int backLog = 15);
	//! Abstract message
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
	//! Message sender task
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

		virtual void executeImplementation(TaskDispatcher::Worker& worker);

		AbstractMessageBroker& _broker;
		std::auto_ptr<TcpSocket> _socketAutoPtr;
		mutable WaitCondition _sendCond;
		bool _terminateFlag;
		MessageQueue _messageQueue;

		friend class SenderTaskTerminator;
		friend class ReceiverTask;
	};
	//! Message receiver task
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

		virtual void executeImplementation(TaskDispatcher::Worker& worker);

		SenderTask& _senderTask;
		AbstractMessageBroker& _broker;
		TcpSocket& _socket;
		bool _terminateFlag;
		mutable ReadWriteLock _terminateFlagRWLock;
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
	//! Thread-safely returns sender task queue size
	inline size_t sendQueueSize() const
	{
		ReadLocker locker(_sendQueueSizeRwLock);
		return _sendQueueSize;
	}
	//! Thread-safely sets sender task queue size
	/*!
	  Subsystem's restart needed to actually apply new value
	  Changes will take place on the next message sending operation
	  \param newValue New sender task queue size
	*/
	inline void setSendQueueSize(size_t newValue)
	{
		WriteLocker locker(_sendQueueSizeRwLock);
		_sendQueueSize = newValue;
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
	inline size_t maxClients() const
	{
		return _taskDispatcher.workersCount() / 2;
	}
	//! Thread-safely sets new maximum clients amount
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param newValue New maximum clients amount
	*/
	inline void setMaxClients(size_t newValue)
	{
		_taskDispatcher.setWorkersCount(newValue * 2);
	}
	//! Thread-safely returns interfaces that should be listen to
	inline std::list<std::string> interfaces() const
	{
		ReadLocker locker(_interfacesRwLock);
		return _interfaces;
	}
	//! Thread-safely sets new interfaces that should be listen to
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param newValue New interfaces that should be listen to
	*/
	inline void setInterfaces(const std::list<std::string>& newValue)
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
	//! Returns true if the broker should terminate itself
	inline bool shouldTerminate() const
	{
		AbstractSubsystem::State brokerState = state();
		return (brokerState != StartingState) && (brokerState != RunningState);
	}

	virtual void start();
	virtual void stop();
protected:
	//! Sender task creation factory virtual method
	/*!
	  \param socket Pointer to TCP-socket
	  \return Pointer to the new sender task object
	*/
	virtual SenderTask * createSenderTask(TcpSocket * socket);
	virtual ReceiverTask * createReceiverTask(SenderTask& senderTask);

	//! Receiving message factory method to override in subclasses
	/*!
	  This method executes in the receiver task's thread
	  \param socket TCP-socket to read the message from
	  \param recieverTask Reference to the reciever task
	  \return Pointer to the new received message or 0 if no message has been received
	*/
	virtual AbstractMessage * recieveMessage(TcpSocket& socket, ReceiverTask& recieverTask) = 0;
	//! Processing message method to override in subclasses
	/*!
	  This method executes in the receiver task's thread
	  \param message Message to process
	  \param recieverTask Reference to the reciever task
	  \param senderTask Reference to the sender task
	*/
	virtual void processMessage(const AbstractMessage& message, ReceiverTask& recieverTask, SenderTask& senderTask) = 0;
	//! Sending message method to override in subclasses
	/*!
	  This method executes in the sender task's thread
	  \param socket TCP-socket to write the message to
	  \param message Message to send
	  \param senderTask Reference to the sender task
	*/
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
			_sleepCond.wait(_broker.timeout());
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
	mutable ReadWriteLock _portRwLock;
	size_t _sendQueueSize;
	mutable ReadWriteLock _sendQueueSizeRwLock;
	Timeout _timeout;
	mutable ReadWriteLock _timeoutRwLock;
	std::list<std::string> _interfaces;
	mutable ReadWriteLock _interfacesRwLock;
	unsigned int _backLog;
	mutable ReadWriteLock _backLogRwLock;
};

} // namespace isl

#endif


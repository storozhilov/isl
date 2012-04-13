#ifndef ISL__ABSTRACT_MESSAGE_BROKER_CONNECTION__HXX
#define ISL__ABSTRACT_MESSAGE_BROKER_CONNECTION__HXX

#include <isl/Core.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/TcpAddrInfo.hxx>
#include <isl/AbstractSubsystem.hxx>
#include <isl/IOError.hxx>
#include <isl/ReadWriteLock.hxx>
#include <isl/MessageQueue.hxx>
#include <isl/MessageBus.hxx>
#include <isl/Error.hxx>
#include <memory>

namespace isl
{

//! Base class for message broker connection
/*!
  TODO Documentation!
*/
template <typename Msg, typename Cloner = CopyMessageCloner<Msg> > class AbstractMessageBrokerConnection : public AbstractSubsystem
{
public:
	typedef MessageQueue<Msg, Cloner> MessageQueueType;
	typedef MessageBus<Msg, Cloner> MessageBusType;

	AbstractMessageBrokerConnection(AbstractSubsystem * owner, const TcpAddrInfo& serverAddrInfo, MessageQueueType& inputQueue,
			const Timeout& listeningInputQueueTimeout = Timeout(0, 100), const Timeout& awaitingConnectionTimeout = Timeout(1)) :
		AbstractSubsystem(owner),
		_serverAddrInfo(serverAddrInfo),
		_inputQueue(inputQueue),
		_listeningInputQueueTimeout(listeningInputQueueTimeout),
		_awaitingConnectionTimeout(awaitingConnectionTimeout),
		_receiverThreadAutoPtr(),
		_senderThreadAutoPtr(),
		_socket(),
		_inputMessageBuses(),
		_outputMessageQueues(),
		_outputMessageBuses()
	{}

	inline TcpSocket& socket()
	{
		return _socket;
	}
	inline MessageQueueType& inputQueue()
	{
		return _inputQueue;
	}
	void addInputMessageBus(MessageBusType& bus)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Input message bus could be added while subsystem idling only"));
		}
		_inputMessageBuses.push_back(&bus);
	}
	void removeInputMessageBus(MessageBusType& bus)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Input message bus could be removed while subsystem idling only"));
		}
		typename MessageBusList::iterator pos = std::find(_inputMessageBuses.begin(), _inputMessageBuses.end(), &bus);
		if (pos == _inputMessageBuses.end()) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Input message bus not found in connection"));
			return;
		}
		_inputMessageBuses.erase(pos);
	}
	void resetInputMessageBuses()
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Input message buses could be reset while subsystem idling only"));
		}
		_inputMessageBuses.clear();
	}
	void addOutputMessageQueue(MessageQueueType& queue)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Output message queue could be added while subsystem idling only"));
		}
		_outputMessageQueues.push_back(&queue);
	}
	void removeOutputMessageQueue(MessageQueueType& queue)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Output message queue could be removed while subsystem idling only"));
		}
		typename MessageQueueList::iterator pos = std::find(_outputMessageQueues.begin(), _outputMessageQueues.end(), &queue);
		if (pos == _outputMessageQueues.end()) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Output message queue not found in connection"));
			return;
		}
		_outputMessageQueues.erase(pos);
	}
	void resetOutputMessageQueues()
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Output message queues could be reset while subsystem idling only"));
		}
		_outputMessageQueues.clear();
	}
	void addOutputMessageBus(MessageBusType& bus)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Output message bus could be added while subsystem idling only"));
		}
		_outputMessageBuses.push_back(&bus);
	}
	void removeOutputMessageBus(MessageBusType& bus)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Output message bus could be removed while subsystem idling only"));
		}
		typename MessageBusList::iterator pos = std::find(_outputMessageBuses.begin(), _outputMessageBuses.end(), &bus);
		if (pos == _outputMessageBuses.end()) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Output message bus not found in connection"));
			return;
		}
		_outputMessageBuses.erase(pos);
	}
	void resetOutputMessageBuses()
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Output message buses could be reset while subsystem idling only"));
		}
		_outputMessageBuses.clear();
	}
	void setServerAddr(const TcpAddrInfo& newValue)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Server address info could be set while subsystem idling only"));
		}
		_serverAddrInfo = newValue;
	}
protected:
	class AbstractReceiverThread : public SubsystemThread
	{
	public:
		AbstractReceiverThread(AbstractMessageBrokerConnection& connection) :
			SubsystemThread(connection),
			_connection(connection),
			_sleepCond()
		{}

		inline TcpSocket& socket()
		{
			return _connection._socket;
		}
	protected:
		virtual void onConnected()
		{
			std::wostringstream msg;
			msg << L"TCP-connection to " << String::utf8Decode(_connection._serverAddrInfo.firstEndpoint().host) << L':' <<
				_connection._serverAddrInfo.firstEndpoint().port << L" has been successfully established";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		}
		virtual void onDisconnected(bool isConnectionAborted)
		{
			std::wostringstream msg;
			msg << L"TCP-connection to " << String::utf8Decode(_connection._serverAddrInfo.firstEndpoint().host) << L':' <<
				_connection._serverAddrInfo.firstEndpoint().port << L" server " << (isConnectionAborted ? L"has been aborted" : L"has been closed");
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		}
		virtual bool onReceiveMessage(const Msg& /*msg*/)
		{
			return true;
		}
		virtual void onSendMessageToQueue(const Msg& /*msg*/, MessageQueueType& /*queue*/)
		{}
		virtual void onSendMessageToBus(const Msg& /*msg*/, MessageBusType& /*bus*/)
		{}
		virtual void onConnectException(std::exception * e = 0)
		{
			std::wostringstream msg;
			msg << L"Connecting to " << String::utf8Decode(_connection._serverAddrInfo.firstEndpoint().host) << L':' <<
				_connection._serverAddrInfo.firstEndpoint().port << L" server ";
			if (e) {
				msg << L"error";
				Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, *e, msg.str()));
			} else {
				msg << L"unknown error";
				Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
		}
		virtual void onReceiveDataException(std::exception * e = 0)
		{
			std::wostringstream msg;
			msg << L"Receiving data from " << String::utf8Decode(_connection._serverAddrInfo.firstEndpoint().host) << L':' <<
				_connection._serverAddrInfo.firstEndpoint().port << L" server ";
			if (e) {
				msg << L"error -> reestablishing connection";
				Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, *e, msg.str()));
			} else {
				msg << L"unknown error -> reestablishing connection";
				Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
		}

		virtual std::auto_ptr<Msg> receiveMessage() = 0;
	private:
		virtual void run()
		{
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Receiver thread has been started"));
			_connection._socket.open();
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Socket has been opened"));
			while (true) {
				if (shouldTerminate()) {
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Message broker connection termination has been detected -> exiting from receiver thread"));
					break;
				}
				if (_connection._socket.connected()) {
					// Receiving message
					std::auto_ptr<Msg> msgAutoPtr;
					try {
						msgAutoPtr = receiveMessage();
					} catch (std::exception& e) {
						onReceiveDataException(&e);
						_connection._socket.close();
						onDisconnected(true);
						_connection._socket.open();
					} catch (...) {
						onReceiveDataException();
						_connection._socket.close();
						onDisconnected(true);
						_connection._socket.open();
					}
					if (msgAutoPtr.get()) {
						std::wostringstream oss;
						oss << L"Message from " << String::utf8Decode(_connection._serverAddrInfo.firstEndpoint().host) << L':' <<
							_connection._serverAddrInfo.firstEndpoint().port << L" server has been received";
						Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
						// Calling on receive message event callback
						if (!onReceiveMessage(*msgAutoPtr.get())) {
							Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Message has been rejected by the on receive event handler"));
							continue;
						}
						// Sending message to all output message queues
						for (typename MessageQueueList::iterator i = _connection._outputMessageQueues.begin(); i != _connection._outputMessageQueues.end(); ++i) {
							if ((*i)->push(*msgAutoPtr.get())) {
								onSendMessageToQueue(*msgAutoPtr.get(), **i);
							}
						}
						// Sending message to all output message buses
						for (typename MessageBusList::iterator i = _connection._outputMessageBuses.begin(); i != _connection._outputMessageBuses.end(); ++i) {
							(*i)->push(*msgAutoPtr.get());
							onSendMessageToBus(*msgAutoPtr.get(), **i);
						}
					}
				} else {
					// Connecting to server
					try {
						_connection._socket.connect(_connection._serverAddrInfo);
						onConnected();
					} catch (std::exception& e) {
						onConnectException(&e);
						MutexLocker locker(_sleepCond.mutex());
						_sleepCond.wait(_connection._awaitingConnectionTimeout);
					} catch (...) {
						onConnectException();
						MutexLocker locker(_sleepCond.mutex());
						_sleepCond.wait(_connection._awaitingConnectionTimeout);
					}
				}
			}
			if (_connection._socket.connected()) {
				_connection._socket.close();
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Socket has been closed"));
				onDisconnected(false);
			}
		}

		AbstractMessageBrokerConnection& _connection;
		WaitCondition _sleepCond;
	};

	class AbstractSenderThread : public SubsystemThread
	{
	public:
		AbstractSenderThread(AbstractMessageBrokerConnection& connection) :
			SubsystemThread(connection),
			_connection(connection),
			_sleepCond()
		{}

		inline TcpSocket& socket()
		{
			return _connection._socket;
		}
	protected:
		virtual bool onReceiveMessage(const Msg& /*msg*/)
		{
			return true;
		}
		virtual void onSendMessage(const Msg& /*msg*/)
		{}
		virtual void onSendDataException(std::exception * e = 0)
		{
			std::wostringstream msg;
			msg << L"Sending data to " << String::utf8Decode(_connection._serverAddrInfo.firstEndpoint().host) << L':' <<
				_connection._serverAddrInfo.firstEndpoint().port << L" server ";
			if (e) {
				msg << L"error";
				Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, *e, msg.str()));
			} else {
				msg << L"unknown error";
				Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
		}

		virtual bool sendMessage(const Msg& msg) = 0;
	private:
		virtual void run()
		{
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Sender thread has been started"));
			std::auto_ptr<Msg> currentMessageAutoPtr;
			bool sendingMessage = false;
			typename MessageBusType::SubscriberListReleaser subscriberListReleaser;
			for (typename MessageBusList::iterator i = _connection._inputMessageBuses.begin(); i != _connection._inputMessageBuses.end(); ++i) {
				std::auto_ptr<typename MessageBusType::Subscriber> subscriberAutoPtr(new typename MessageBusType::Subscriber(**i, _connection._inputQueue));
				subscriberListReleaser.addSubscriber(subscriberAutoPtr.get());
				subscriberAutoPtr.release();
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Sender thread's input queue  has been subscribed to the input message bus"));
			}
			while (true) {
				if (shouldTerminate()) {
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Message broker connection termination has been detected -> exiting from sender thread"));
					return;
				}
				if (sendingMessage) {
					if (_connection._socket.connected()) {
						// Sending message to peer
						bool messageSent = false;
						try {
							messageSent = sendMessage(*currentMessageAutoPtr.get());
						} catch (std::exception& e) {
							onSendDataException(&e);
							MutexLocker locker(_sleepCond.mutex());
							_sleepCond.wait(_connection._awaitingConnectionTimeout);
							continue;
						} catch (...) {
							onSendDataException();
							MutexLocker locker(_sleepCond.mutex());
							_sleepCond.wait(_connection._awaitingConnectionTimeout);
							continue;
						}
						if (messageSent) {
							sendingMessage = false;
							std::wostringstream oss;
							oss << L"Message to " << String::utf8Decode(_connection._serverAddrInfo.firstEndpoint().host) << L':'
								<< _connection._serverAddrInfo.firstEndpoint().port << L" server has been sent";
							Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
							onSendMessage(*currentMessageAutoPtr.get());
						}
					} else {
						// Waiting for socket to be connected
						MutexLocker locker(_sleepCond.mutex());
						_sleepCond.wait(_connection._awaitingConnectionTimeout);
					}
				} else {
					// Fetching message from the bus
					currentMessageAutoPtr = _connection._inputQueue.pop(_connection._listeningInputQueueTimeout);
					if (currentMessageAutoPtr.get()) {
						Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Message has been fetched from the input queue"));
						if (onReceiveMessage(*currentMessageAutoPtr.get())) {
							sendingMessage = true;
						} else {
							Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Message has been rejected by the on receive event handler"));
						}
					}
				}
			}
		}

		AbstractMessageBrokerConnection& _connection;
		WaitCondition _sleepCond;
	};

	virtual void beforeStart()
	{
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Opening a socket"));
		if (!_receiverThreadAutoPtr.get()) {
			_receiverThreadAutoPtr = createReceiverThread();
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Receiver thread has been created"));
		}
		if (!_senderThreadAutoPtr.get()) {
			_senderThreadAutoPtr = createSenderThread();
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Sender thread has been created"));
		}
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Starting message broker connection"));
	}
	virtual void afterStart()
	{
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Message broker connection has been started"));
	}
	virtual void beforeStop()
	{
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Stopping message broker connection"));
	}
	virtual void afterStop()
	{
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Message broker connection has been stopped"));
	}

	virtual std::auto_ptr<AbstractReceiverThread> createReceiverThread() = 0;
	virtual std::auto_ptr<AbstractSenderThread> createSenderThread() = 0;
private:
	AbstractMessageBrokerConnection();
	AbstractMessageBrokerConnection(const AbstractMessageBrokerConnection&);						// No copy

	AbstractMessageBrokerConnection& operator=(const AbstractMessageBrokerConnection&);					// No copy

	typedef std::list<MessageQueueType *> MessageQueueList;
	typedef std::list<MessageBusType *> MessageBusList;

	TcpAddrInfo _serverAddrInfo;
	MessageQueueType& _inputQueue;
	const Timeout _listeningInputQueueTimeout;
	const Timeout _awaitingConnectionTimeout;
	std::auto_ptr<AbstractReceiverThread> _receiverThreadAutoPtr;
	std::auto_ptr<AbstractSenderThread> _senderThreadAutoPtr;
	TcpSocket _socket;
	MessageBusList _inputMessageBuses;
	MessageQueueList _outputMessageQueues;
	MessageBusList _outputMessageBuses;
};

} // namespace isl

#endif

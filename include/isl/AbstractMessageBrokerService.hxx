#ifndef ISL__ABSTRACT_MESSAGE_BROKER_SERVICE__HXX
#define ISL__ABSTRACT_MESSAGE_BROKER_SERVICE__HXX

#include <isl/AbstractAsyncTcpService.hxx>

namespace isl
{

//! Base class for message broker service
/*!
  TODO Documentation!
*/
template <typename Msg, typename Cloner = CopyMessageCloner<Msg> > class AbstractMessageBrokerService : public AbstractAsyncTcpService
{
public:
	typedef MessageQueue<Msg, Cloner> MessageQueueType;
	typedef MessageBus<Msg, Cloner> MessageBusType;

	AbstractMessageBrokerService(AbstractSubsystem * owner, size_t maxClients, const isl::Timeout& listeningInputQueueTimeout = isl::Timeout(0, 100),
			size_t maxTaskQueueOverflowSize = 0) :
		AbstractAsyncTcpService(owner, maxClients, maxTaskQueueOverflowSize),
		_listeningInputQueueTimeout(listeningInputQueueTimeout),
		_inputMessageBuses(),
		_outputMessageQueues(),
		_outputMessageBuses()
	{}

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
protected:
	class Connection : public AbstractAsyncTcpService::Connection
	{
	public:
		Connection(AbstractMessageBrokerService& service, TcpSocket * socketPtr) :
			AbstractAsyncTcpService::Connection(socketPtr),
			_inputMessageQueueAutoPtr(service.createInputMessageQueue())
		{}

		inline MessageQueueType& inputMessageQueue()
		{
			return *_inputMessageQueueAutoPtr.get();
		}
	private:
		std::auto_ptr<MessageQueueType> _inputMessageQueueAutoPtr;
	};

	class AbstractReceiverTask : public AbstractAsyncTcpService::AbstractReceiverTask
	{
	public:
		AbstractReceiverTask(AbstractMessageBrokerService& service, Connection * connection) :
			AbstractAsyncTcpService::AbstractReceiverTask(service, connection),
			_service(service),
			_connection(connection)
		{}

		inline AbstractMessageBrokerService& service()
		{
			return _service;
		}
		inline Connection& connection()
		{
			return *_connection;
		}
		inline MessageQueueType& inputMessageQueue()
		{
			return _connection->inputMessageQueue();
		}
	protected:
		virtual bool onReceiveMessage(const Msg& /*msg*/)
		{
			return true;
		}
		virtual void onSendMessageToQueue(const Msg& /*msg*/, MessageQueueType& /*queue*/)
		{}
		virtual void onSendMessageToBus(const Msg& /*msg*/, MessageBusType& /*bus*/)
		{}
		virtual void onReceiveDataException(std::exception * e = 0)
		{
			std::wostringstream msg;
			msg << L"Receiving data from " << String::utf8Decode(socket().remoteAddr().firstEndpoint().host) << L':' <<
				socket().remoteAddr().firstEndpoint().port << L" client ";
			if (e) {
				msg << L"error -> exiting from task execution";
				Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, *e, msg.str()));
			} else {
				msg << L"unknown error -> exiting from task execution";
				Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
		}

		virtual std::auto_ptr<Msg> receiveMessage() = 0;
	private:
		virtual void execute(TaskDispatcherType::WorkerThread& worker)
		{
			isl::Core::debugLog.log(isl::DebugLogMessage(SOURCE_LOCATION_ARGS, L"Receiver task execution has been started"));
			while (true) {
				if (shouldTerminate()) {
					isl::Core::debugLog.log(isl::DebugLogMessage(SOURCE_LOCATION_ARGS, L"Client service termination has been detected -> exiting from receiver task execution"));
					return;
				}
				if (!socket().connected()) {
					isl::Core::debugLog.log(isl::DebugLogMessage(SOURCE_LOCATION_ARGS, L"Client connection socket is not connected -> exiting from receiver task execution"));
					return;
				}
				try {
					std::auto_ptr<Msg> msgAutoPtr = receiveMessage();
					if (msgAutoPtr.get()) {
						std::wostringstream oss;
						oss << L"Message from " << String::utf8Decode(socket().remoteAddr().firstEndpoint().host) << L':' << socket().remoteAddr().firstEndpoint().port << L" client has been received";
						Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
						// Calling on receive message event callback
						if (!onReceiveMessage(*msgAutoPtr.get())) {
							Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Message has been rejected by the on receive event handler"));
							continue;
						}
						// Sending message to all output message queues
						for (typename MessageQueueList::iterator i = _service._outputMessageQueues.begin(); i != _service._outputMessageQueues.end(); ++i) {
							if ((*i)->push(*msgAutoPtr.get())) {
								onSendMessageToQueue(*msgAutoPtr.get(), **i);
							}
						}
						// Sending message to all output message buses
						for (typename MessageBusList::iterator i = _service._outputMessageBuses.begin(); i != _service._outputMessageBuses.end(); ++i) {
							(*i)->push(*msgAutoPtr.get());
							onSendMessageToBus(*msgAutoPtr.get(), **i);
						}
					}
				} catch (std::exception& e) {
					onReceiveDataException(&e);
					return;
				} catch (...) {
					onReceiveDataException();
					return;
				}
			}
		}

		AbstractMessageBrokerService& _service;
		Connection * _connection;
	};

	class AbstractSenderTask : public AbstractAsyncTcpService::AbstractSenderTask
	{
	public:
		AbstractSenderTask(AbstractMessageBrokerService& service, Connection * connection) :
			AbstractAsyncTcpService::AbstractSenderTask(service, connection),
			_service(service),
			_connection(connection)
		{}

		inline AbstractMessageBrokerService& service()
		{
			return _service;
		}
		inline Connection& connection()
		{
			return *_connection;
		}
		inline MessageQueueType& inputMessageQueue()
		{
			return _connection->inputMessageQueue();
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
			msg << L"Sending data to " << String::utf8Decode(socket().remoteAddr().firstEndpoint().host) << L':' <<
				socket().remoteAddr().firstEndpoint().port << L" server ";
			if (e) {
				msg << L"error -> exiting from task execution";
				Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, *e, msg.str()));
			} else {
				msg << L"unknown error -> exiting from task execution";
				Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
		}

		virtual bool sendMessage(const Msg& msg) = 0;
	private:
		virtual void execute(TaskDispatcherType::WorkerThread& worker)
		{
			isl::Core::debugLog.log(isl::DebugLogMessage(SOURCE_LOCATION_ARGS, L"Sender task execution has been started"));
			std::auto_ptr<asd::Message> currentMessageAutoPtr;
			bool sendingMessage = false;
			typename MessageBusType::SubscriberListReleaser subscriberListReleaser;
			for (typename MessageBusList::iterator i = _service._inputMessageBuses.begin(); i != _service._inputMessageBuses.end(); ++i) {
				std::auto_ptr<typename MessageBusType::Subscriber> subscriberAutoPtr(new typename MessageBusType::Subscriber(**i, inputMessageQueue()));
				subscriberListReleaser.addSubscriber(subscriberAutoPtr.get());
				subscriberAutoPtr.release();
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Sender task's input queue has been subscribed to the input message bus"));
			}
			while (true) {
				if (shouldTerminate()) {
					isl::Core::debugLog.log(isl::DebugLogMessage(SOURCE_LOCATION_ARGS, L"Client service termination has been detected -> exiting from sender task execution"));
					return;
				}
				if (!socket().connected()) {
					isl::Core::debugLog.log(isl::DebugLogMessage(SOURCE_LOCATION_ARGS, L"Client connection socket is not connected -> exiting from sender task execution"));
					return;
				}
				if (sendingMessage) {
					// Sending message to peer
					bool messageSent = false;
					try {
						messageSent = sendMessage(*currentMessageAutoPtr.get());
					} catch (std::exception& e) {
						onSendDataException(&e);
						return;
					} catch (...) {
						onSendDataException();
						return;
					}
					if (messageSent) {
						sendingMessage = false;
						std::wostringstream oss;
						oss << L"Message to " << String::utf8Decode(socket().remoteAddr().firstEndpoint().host) << L':' << socket().remoteAddr().firstEndpoint().port << L" client has been sent";
						Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
						onSendMessage(*currentMessageAutoPtr.get());
					}
				} else {
					// Fetching message from the bus
					currentMessageAutoPtr = inputMessageQueue().pop(_service._listeningInputQueueTimeout);
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

		AbstractMessageBrokerService& _service;
		Connection * _connection;
	};

	//! Creating connection factory method
	/*!
	  \param socket TCP-socket for collaborative usage
	  \return std::auto_ptr with new connection
	*/
	virtual std::auto_ptr<AbstractAsyncTcpService::Connection> createConnection(TcpSocket * socket)
	{
		return std::auto_ptr<AbstractAsyncTcpService::Connection>(new Connection(*this, socket));
	}
	//! Creating input connection's message queue factory method
	virtual std::auto_ptr<MessageQueueType> createInputMessageQueue()
	{
		return std::auto_ptr<MessageQueueType>(new MessageQueueType());
	}
private:
	AbstractMessageBrokerService();
	AbstractMessageBrokerService(const AbstractMessageBrokerService&);						// No copy

	AbstractMessageBrokerService& operator=(const AbstractMessageBrokerService&);					// No copy

	typedef std::list<MessageQueueType *> MessageQueueList;
	typedef std::list<MessageBusType *> MessageBusList;

	const Timeout _listeningInputQueueTimeout;
	MessageBusList _inputMessageBuses;
	MessageQueueList _outputMessageQueues;
	MessageBusList _outputMessageBuses;
};

} // namespace isl

#endif
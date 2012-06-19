#ifndef ISL__ABSTRACT_MESSAGE_BROKER_SERVICE__HXX
#define ISL__ABSTRACT_MESSAGE_BROKER_SERVICE__HXX

#include <isl/common.hxx>
#include <isl/AbstractAsyncTcpService.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>

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
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Input message bus could be added while subsystem idling only"));
		}
		_inputMessageBuses.push_back(&bus);
	}
	void removeInputMessageBus(MessageBusType& bus)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Input message bus could be removed while subsystem idling only"));
		}
		typename MessageBusList::iterator pos = std::find(_inputMessageBuses.begin(), _inputMessageBuses.end(), &bus);
		if (pos == _inputMessageBuses.end()) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Input message bus not found in connection"));
			return;
		}
		_inputMessageBuses.erase(pos);
	}
	void resetInputMessageBuses()
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Input message buses could be reset while subsystem idling only"));
		}
		_inputMessageBuses.clear();
	}
	void addOutputMessageQueue(MessageQueueType& queue)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Output message queue could be added while subsystem idling only"));
		}
		_outputMessageQueues.push_back(&queue);
	}
	void removeOutputMessageQueue(MessageQueueType& queue)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Output message queue could be removed while subsystem idling only"));
		}
		typename MessageQueueList::iterator pos = std::find(_outputMessageQueues.begin(), _outputMessageQueues.end(), &queue);
		if (pos == _outputMessageQueues.end()) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Output message queue not found in connection"));
			return;
		}
		_outputMessageQueues.erase(pos);
	}
	void resetOutputMessageQueues()
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Output message queues could be reset while subsystem idling only"));
		}
		_outputMessageQueues.clear();
	}
	void addOutputMessageBus(MessageBusType& bus)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Output message bus could be added while subsystem idling only"));
		}
		_outputMessageBuses.push_back(&bus);
	}
	void removeOutputMessageBus(MessageBusType& bus)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Output message bus could be removed while subsystem idling only"));
		}
		typename MessageBusList::iterator pos = std::find(_outputMessageBuses.begin(), _outputMessageBuses.end(), &bus);
		if (pos == _outputMessageBuses.end()) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Output message bus not found in connection"));
			return;
		}
		_outputMessageBuses.erase(pos);
	}
	void resetOutputMessageBuses()
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Output message buses could be reset while subsystem idling only"));
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
			std::ostringstream msg;
			msg << "Receiving data from " << socket().remoteAddr().firstEndpoint().host << ':' <<
				socket().remoteAddr().firstEndpoint().port << " client ";
			if (e) {
				msg << "error -> exiting from task execution";
				errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, *e, msg.str()));
			} else {
				msg << "unknown error -> exiting from task execution";
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
		}

		virtual std::auto_ptr<Msg> receiveMessage() = 0;
	private:
		virtual void execute(TaskDispatcherType::WorkerThread& worker)
		{
			isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Receiver task execution has been started"));
			while (true) {
				if (shouldTerminate()) {
					isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Client service termination has been detected -> exiting from receiver task execution"));
					return;
				}
				if (!socket().connected()) {
					isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Client connection socket is not connected -> exiting from receiver task execution"));
					return;
				}
				try {
					std::auto_ptr<Msg> msgAutoPtr = receiveMessage();
					if (msgAutoPtr.get()) {
						std::ostringstream oss;
						oss << "Message from " << socket().remoteAddr().firstEndpoint().host << ':' << socket().remoteAddr().firstEndpoint().port << " client has been received";
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
						// Calling on receive message event callback
						if (!onReceiveMessage(*msgAutoPtr.get())) {
							debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by the on receive event handler"));
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
			std::ostringstream msg;
			msg << "Sending data to " << socket().remoteAddr().firstEndpoint().host << ':' <<
				socket().remoteAddr().firstEndpoint().port << " server ";
			if (e) {
				msg << "error -> exiting from task execution";
				errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, *e, msg.str()));
			} else {
				msg << "unknown error -> exiting from task execution";
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
		}

		virtual bool sendMessage(const Msg& msg) = 0;
	private:
		virtual void execute(TaskDispatcherType::WorkerThread& worker)
		{
			isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Sender task execution has been started"));
			std::auto_ptr<Msg> currentMessageAutoPtr;
			bool sendingMessage = false;
			typename MessageBusType::SubscriberListReleaser subscriberListReleaser;
			for (typename MessageBusList::iterator i = _service._inputMessageBuses.begin(); i != _service._inputMessageBuses.end(); ++i) {
				std::auto_ptr<typename MessageBusType::Subscriber> subscriberAutoPtr(new typename MessageBusType::Subscriber(**i, inputMessageQueue()));
				subscriberListReleaser.addSubscriber(subscriberAutoPtr.get());
				subscriberAutoPtr.release();
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Sender task's input queue has been subscribed to the input message bus"));
			}
			while (true) {
				if (shouldTerminate()) {
					isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Client service termination has been detected -> exiting from sender task execution"));
					return;
				}
				if (!socket().connected()) {
					isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Client connection socket is not connected -> exiting from sender task execution"));
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
						std::ostringstream oss;
						oss << "Message to " << socket().remoteAddr().firstEndpoint().host << ':' << socket().remoteAddr().firstEndpoint().port << " client has been sent";
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
						onSendMessage(*currentMessageAutoPtr.get());
					}
				} else {
					// Fetching message from the bus
					currentMessageAutoPtr = inputMessageQueue().pop(_service._listeningInputQueueTimeout);
					if (currentMessageAutoPtr.get()) {
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been fetched from the input queue"));
						if (onReceiveMessage(*currentMessageAutoPtr.get())) {
							sendingMessage = true;
						} else {
							debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by the on receive event handler"));
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

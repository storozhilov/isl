#ifndef ISL__ABSTRACT_MESSAGE_BROKER_SERVICE__HXX
#define ISL__ABSTRACT_MESSAGE_BROKER_SERVICE__HXX

#include <isl/common.hxx>
#include <isl/AbstractAsyncTcpService.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>
#include <isl/MessageProvider.hxx>
#include <isl/MessageBuffer.hxx>

namespace isl
{

//! Message broker service subsystem abstract templated class
/*!
  Use this class for your message broker service implementation. It creates (maxClients * 2) worker threads
  to execute 2 tasks per each client. These tasks should be objects of the descendants of the following abstract classes:

  - AbstractMessageBrokerService::AbstractReceiverTask - is for receiving messages from the transport
    and providing them to subscribed message consumers. Your descendant of this class should override
    AbstractMessageBrokerService::AbstractReceiverTask::receiveMessage() method, which actually
    receives message from the network transport;
  - AbstractMessageBrokerService::AbstractSenderTask - is for consuming messages from the message
    providers subscribed to and sending them to the transport. Your descendant of this class should override
    AbstractMessageBrokerService::AbstractSenderTask::sendMessage() method, which actually
    sends message to the network transport.

  To implement your message broker service class you should also override following abstract factory
  methods, which are used for task objects creation during new connection service:

  - AbstractAsyncTcpService::createReceiverTask() - creates receiver task object;
  - AbstractAsyncTcpService::createSenderTask() - creates sender task object.

  \tparam Msg Message class with <tt>Msg * Msg::clone() const</tt> method
*/
template <typename Msg> class AbstractMessageBrokerService : public AbstractAsyncTcpService
{
public:
	typedef Msg MessageType;
	typedef MessageProvider<Msg> MessageProviderType;
	typedef AbstractMessageConsumer<Msg> AbstractMessageConsumerType;
	typedef MessageQueue<Msg> MessageQueueType;
	typedef MessageBuffer<Msg> MessageBufferType;

	//! Constructor
	/*!
	  \param owner Pointer to the owner subsystem
	  \param maxClients Maximum clients amount
	  \param listeningInputQueueTimeout Listening input queue timeout
	  \param maxTaskQueueOverflowSize Maximum tasks queue overflow size
	*/
	AbstractMessageBrokerService(AbstractSubsystem * owner, size_t maxClients, const isl::Timeout& listeningInputQueueTimeout = isl::Timeout(0, 100),
			size_t maxTaskQueueOverflowSize = 0) :
		AbstractAsyncTcpService(owner, maxClients, maxTaskQueueOverflowSize),
		_listeningInputQueueTimeout(listeningInputQueueTimeout),
		_providers(),
		_consumers()
	{}
	//! Adds message provider to subscribe input queue to while running
	/*!
	  \param provider Reference to provider to add
	*/
	void addProvider(MessageProviderType& provider)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Message provider could be added while subsystem idling only"));
		}
		_providers.push_back(&provider);
	}
	//! Removes message provider
	/*!
	  \param provider Reference to provider to remove
	*/
	void removeProvider(MessageProviderType& provider)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Message provider  could be removed while subsystem idling only"));
		}
		typename ProvidersContainer::iterator pos = std::find(_providers.begin(), _providers.end(), &provider);
		if (pos == _providers.end()) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message provider not found in connection"));
			return;
		}
		_providers.erase(pos);
	}
	//! Removes all message providers
	void resetProviders()
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Message providers could be reset while subsystem idling only"));
		}
		_providers.clear();
	}
	//! Adds message consumer for providing incoming messages to while running
	/*!
	  \param consumer Reference to consumer to add
	*/
	void addConsumer(AbstractMessageConsumerType& consumer)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Message consumer could be added while subsystem idling only"));
		}
		_consumers.push_back(&consumer);
	}
	//! Removes message consumer
	/*!
	  \param consumer Reference to consumer to remove
	*/
	void removeConsumer(AbstractMessageConsumerType& consumer)
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Message consumer  could be removed while subsystem idling only"));
		}
		typename ConsumersContainer::iterator pos = std::find(_consumers.begin(), _consumers.end(), &consumer);
		if (pos == _consumers.end()) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message consumer not found in connection"));
			return;
		}
		_consumers.erase(pos);
	}
	//! Removes all message consumers
	void resetConsumers()
	{
		MutexLocker locker(startStopMutex());
		if (state() != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Message consumers could be reset while subsystem idling only"));
		}
		_consumers.clear();
	}
protected:
	//! Class for storing data shared between receiver and sender task objects
	class Connection : public AbstractAsyncTcpService::Connection
	{
	public:
		//! Constructor
		/*!
		  \param service Reference to message broker service
		  \param socketPtr Pointer to client connection socket
		*/
		Connection(AbstractMessageBrokerService& service, TcpSocket * socketPtr) :
			AbstractAsyncTcpService::Connection(socketPtr),
			_inputQueueAutoPtr(service.createInputQueue())
		{}
		//! Returns a reference to input message queue
		inline MessageQueueType& inputQueue()
		{
			return *_inputQueueAutoPtr.get();
		}
	private:
		std::auto_ptr<MessageQueueType> _inputQueueAutoPtr;
	};
	//! Receiver task object abstract class
	class AbstractReceiverTask : public AbstractAsyncTcpService::AbstractReceiverTask
	{
	public:
		//! Constructor
		/*!
		  \param service Reference to message broker service
		  \param Pointer to the connection object
		*/
		AbstractReceiverTask(AbstractMessageBrokerService& service, Connection * connection) :
			AbstractAsyncTcpService::AbstractReceiverTask(service, connection),
			_service(service),
			_connection(connection)
		{}
		//! Returns a reference to the message broker service
		inline AbstractMessageBrokerService& service()
		{
			return _service;
		}
		//! Returns a reference to the connection object
		inline Connection& connection()
		{
			return *_connection;
		}
		//! Returns a reference to the input message queue
		inline MessageQueueType& inputQueue()
		{
			return _connection->inputQueue();
		}
	protected:
		//! On receive message from transport event handler
		/*!
		  Default implementation records an entry to the ISL's debug log and returns true

		  \param msg Constant reference to the received message
		  \return True if to proceed with the message or false to discard it
		*/
		virtual bool onReceiveMessage(const Msg& /*msg*/)
		{
			std::ostringstream oss;
			oss << "Message has been received from the " << socket().remoteAddr().firstEndpoint().host << ':' << socket().remoteAddr().firstEndpoint().port << " client";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
			return true;
		}
		//! On provide incoming message to the consumer event handler
		/*!
		  Default implementation records an entry to the ISL's debug log

		  \param msg Constant reference to the provided message
		  \param consumer Reference to the message consumer where the message has been provided to
		*/
		virtual void onProvideMessage(const Msg& msg, AbstractMessageConsumerType& consumer)
		{
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been provided to the consumer"));
		}
		//! On receive data from transport exception event handler
		/*!
		  Default implementation records an entry to the ISL's error log

		  \param e Pointer to std::exception instance or 0 if exception is unknown
		*/
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
		//! Receiving message from transport abstract method
		/*!
		  \return Auto-pointer to the received message or to 0 if no message have been received
		*/
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
						// Calling on receive message event callback
						if (!onReceiveMessage(*msgAutoPtr.get())) {
							debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by the on receive event handler"));
							continue;
						}
						// Providing message to all consumers
						for (typename ConsumersContainer::iterator i = _service._consumers.begin(); i != _service._consumers.end(); ++i) {
							if ((*i)->push(*msgAutoPtr.get())) {
								onProvideMessage(*msgAutoPtr.get(), **i);
							}
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
	//! Sender task object abstract class
	class AbstractSenderTask : public AbstractAsyncTcpService::AbstractSenderTask
	{
	public:
		//! Constructor
		/*!
		  \param service Reference to message broker service
		  \param Pointer to the connection object
		*/
		AbstractSenderTask(AbstractMessageBrokerService& service, Connection * connection) :
			AbstractAsyncTcpService::AbstractSenderTask(service, connection),
			_service(service),
			_connection(connection),
			_consumeBuffer()
		{}

		//! Returns a reference to the message broker service
		inline AbstractMessageBrokerService& service()
		{
			return _service;
		}
		//! Returns a reference to the connection object
		inline Connection& connection()
		{
			return *_connection;
		}
		//! Returns a reference to the input message queue
		inline MessageQueueType& inputQueue()
		{
			return _connection->inputQueue();
		}
	protected:
		//! On consume message from any provider event handler
		/*!
		  Default implementation records an entry to the ISL's debug log and returns true

		  \param msg Constant reference to the consumed message
		  \return True if to proceed with the message or false to discard it
		*/
		virtual bool onConsumeMessage(const Msg& /*msg*/)
		{
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been fetched from the consume buffer"));
			return true;
		}
		//! On send message to transport event handler
		/*!
		  Default implementation records an entry to the ISL's debug log

		  \param Constant reference to the sent message
		*/
		virtual void onSendMessage(const Msg& /*msg*/)
		{
			std::ostringstream oss;
			oss << "Message has been sent to " << socket().remoteAddr().firstEndpoint().host << ':' << socket().remoteAddr().firstEndpoint().port << " client";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
		}
		//! On send data to transport exception event handler
		/*!
		  Default implementation records an entry to the ISL's error log

		  \param e Pointer to std::exception instance or 0 if exception is unknown
		*/
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
		//! Sending message to transport abstract method
		/*!
		  \param msg Constant reference to message to send
		  \return True if the message has been sent
		*/
		virtual bool sendMessage(const Msg& msg) = 0;
	private:
		virtual void execute(TaskDispatcherType::WorkerThread& worker)
		{
			isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Sender task execution has been started"));
			std::auto_ptr<Msg> currentMessageAutoPtr;
			bool sendingMessage = false;
			// Susbcribing input message queue to the providers
			typename MessageProviderType::SubscriberListReleaser subscriberListReleaser;
			for (typename ProvidersContainer::iterator i = _service._providers.begin(); i != _service._providers.end(); ++i) {
				std::auto_ptr<typename MessageProviderType::Subscriber> subscriberAutoPtr(new typename MessageProviderType::Subscriber(**i, inputQueue()));
				subscriberListReleaser.addSubscriber(subscriberAutoPtr.get());
				subscriberAutoPtr.release();
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Sender task's input queue has been subscribed to the message provider"));
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
						onSendMessage(*currentMessageAutoPtr.get());
					}
				} else if (_consumeBuffer.empty()) {
					// Fetching all messages from the input to the consume buffer
					size_t consumedMessagesAmount = inputQueue().popAll(_consumeBuffer, _service._listeningInputQueueTimeout);
					if (consumedMessagesAmount > 0) {
						std::ostringstream oss;
						oss << consumedMessagesAmount << " message(s) has been fetched from the input queue to the consume buffer";
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
					}
				} else {
					// Fetching next message from the consume buffer
					currentMessageAutoPtr = _consumeBuffer.pop();
					if (onConsumeMessage(*currentMessageAutoPtr.get())) {
						sendingMessage = true;
					} else {
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by the on consume event handler"));
					}
				}
			}
		}

		AbstractMessageBrokerService& _service;
		Connection * _connection;
		MessageBufferType _consumeBuffer;
	};

	//! Connection creation factory method
	/*!
	  \param socket TCP-socket for collaborative usage
	  \return std::auto_ptr with new connection
	*/
	virtual std::auto_ptr<AbstractAsyncTcpService::Connection> createConnection(TcpSocket * socket)
	{
		return std::auto_ptr<AbstractAsyncTcpService::Connection>(new Connection(*this, socket));
	}
	//! Connection's input message queue creation factory method
	virtual std::auto_ptr<MessageQueueType> createInputQueue()
	{
		return std::auto_ptr<MessageQueueType>(new MessageQueueType());
	}
private:
	AbstractMessageBrokerService();
	AbstractMessageBrokerService(const AbstractMessageBrokerService&);						// No copy

	AbstractMessageBrokerService& operator=(const AbstractMessageBrokerService&);					// No copy

	typedef std::list<MessageProviderType *> ProvidersContainer;
	typedef std::list<AbstractMessageConsumerType *> ConsumersContainer;

	const Timeout _listeningInputQueueTimeout;
	ProvidersContainer _providers;
	ConsumersContainer _consumers;
};

} // namespace isl

#endif

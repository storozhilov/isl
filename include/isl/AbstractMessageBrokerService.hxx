#ifndef ISL__ABSTRACT_MESSAGE_BROKER_SERVICE__HXX
#define ISL__ABSTRACT_MESSAGE_BROKER_SERVICE__HXX

#include <isl/AbstractAsyncTcpService.hxx>
#include <isl/StateSet.hxx>
#include <isl/Ticker.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>
#include <isl/MessageProvider.hxx>
#include <isl/MessageBus.hxx>
#include <isl/MessageQueue.hxx>
#include <isl/MessageBuffer.hxx>

namespace isl
{

//! Message broker service subsystem abstract templated class
/*!
  Use this class for your message broker service implementation. It creates (maxClients * 2) worker threads
  per each client connection: one is for receiving of the messages and another is for sending ones. You should
  subclass a AbstractMessageBrokerService::AbstractTask with following abstract virtual methods to be overriden:

  - AbstractMessageBrokerService::AbstractTask::receiveMessage() - for receiving message from the client
  - AbstractMessageBrokerService::AbstractTask::sendMessage() - for receiving message to the client

  You should also override an AbstractAsyncTcpService::createTask() client connection task creation factory method.

  Client connection task execution terminates if an Exception with TcpSocket::ConnectionAbortedError error
  has been thrown during AbstractMessageBrokerService::AbstractTask::receiveMessage() or
  AbstractMessageBrokerService::AbstractTask::sendMessage() methods execution. You can terminate
  client connection task implicitly by calling
  AbstractMessageBrokerService::AbstractTask::appointTermination() method.

  \tparam Msg Message class
  \tparam Cloner Message cloner class with static <tt>Msg * Cloner::clone(const Msg& msg)</tt> method for cloning the message
*/
template <typename Msg, typename Cloner = CopyMessageCloner<Msg> > class AbstractMessageBrokerService : public AbstractAsyncTcpService
{
public:
	typedef Msg MessageType;							//!< Message type
	typedef MessageProvider<MessageType> MessageProviderType;			//!< Message provider type
	typedef AbstractMessageConsumer<MessageType> AbstractMessageConsumerType;	//!< Abstract message consumer type
	typedef MessageQueue<MessageType, Cloner> MessageQueueType;			//!< Message queue type
	typedef MessageBuffer<MessageType, Cloner> MessageBufferType;			//!< Message buffer type
	typedef MessageBus<MessageType> MessageBusType;					//!< Message bus type

	//! Constructor
	/*!
	  \param owner Pointer to the owner subsystem
	  \param maxClients Maximum clients amount
	  \param clockTimeout Subsystem's clock timeout
	*/
	AbstractMessageBrokerService(Subsystem * owner, size_t maxClients, const Timeout& clockTimeout = Timeout::defaultTimeout()) :
		AbstractAsyncTcpService(owner, maxClients, clockTimeout),
		_providers(),
		_consumers()
	{}
	//! Adds message provider to subscribe input queue to while running
	/*!
	  \param provider Reference to provider to add

	  \note Thread-unsafe: call it when subsystem is idling only: call it when subsystem is idling only
	*/
	void addProvider(MessageProviderType& provider)
	{
		_providers.push_back(&provider);
	}
	//! Removes message provider
	/*!
	  \param provider Reference to provider to remove

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	void removeProvider(MessageProviderType& provider)
	{
		typename ProvidersContainer::iterator pos = std::find(_providers.begin(), _providers.end(), &provider);
		if (pos == _providers.end()) {
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Message provider not found"));
			return;
		}
		_providers.erase(pos);
	}
	//! Removes all message providers
	/*!
	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	inline void resetProviders()
	{
		_providers.clear();
	}
	//! Adds message consumer for providing incoming messages to while running
	/*!
	  \param consumer Reference to consumer to add

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	void addConsumer(AbstractMessageConsumerType& consumer)
	{
		_consumers.push_back(&consumer);
	}
	//! Removes message consumer
	/*!
	  \param consumer Reference to consumer to remove

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	void removeConsumer(AbstractMessageConsumerType& consumer)
	{
		typename ConsumersContainer::iterator pos = std::find(_consumers.begin(), _consumers.end(), &consumer);
		if (pos == _consumers.end()) {
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Message consumer not found"));
			return;
		}
		_consumers.erase(pos);
	}
	//! Removes all message consumers
	/*!
	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	void resetConsumers()
	{
		_consumers.clear();
	}
protected:
	//! Client connection task abstract class
	class AbstractTask : public AbstractAsyncTcpService::AbstractTask
	{
	public:
		//! Constructor
		/*!
		  \param socket Reference to the client connection socket
		*/
		AbstractTask(AbstractMessageBrokerService& service, TcpSocket& socket) :
			AbstractAsyncTcpService::AbstractTask(socket),
			_service(service),
			_stateSet(),
			_consumeBuffer(),
			_inputQueueAutoPtr(service.createInputQueue(*this)),
			_outputBusAutoPtr(service.createOutputBus(*this))
		{}
		//! Inspects if the task execution should be terminated
		bool shouldTerminate()
		{
			StateSetType::SetType set = _stateSet.fetch();
			return set.find(TerminationState) != set.end();
		}
		//! Appoints a task execution termination
		/*!
		  \param newValue New should terminate flag's value
		*/
		void appointTermination()
		{
			_stateSet.insert(TerminationState);
		}
		//! Returns a reference to the internal input message queue
		inline MessageQueueType& inputQueue()
		{
			return *_inputQueueAutoPtr.get();
		}
		//! Returns a reference to the internal output message bus
		inline MessageBusType& outputBus()
		{
			return *_outputBusAutoPtr.get();
		}
		//! Sends a request message to message broker client and waits for the response(-s)
		/*!
		  \param request Constant reference to the request message to send
		  \param responseQueue Reference to response-filtering message queue to save a response(-s) to
		  \param limit Time limit to wait for response
		  \return TRUE if the request has been accepted by the input message queue and the response(-s) has been fetched during timeout
		*/
		inline bool sendRequest(const MessageType& request, MessageQueueType& responseQueue, const Timestamp& limit)
		{
			responseQueue.clear();
			typename MessageProviderType::Subscriber subscriber(outputBus(), responseQueue);
			if (!inputQueue().push(request)) {
				return false;
			}
			return responseQueue.await(limit);
		}
	protected:
		//! Returns reference to the state set of the task
		StateSetType& stateSet()
		{
			return _stateSet;
		}
		//! Before receiver execution event handler
		virtual void beforeExecuteReceive()
		{}
		//! On receive message thread overload event handler
		/*!
		  Default implementation does nothing and returns TRUE.
		  \param Ticks expired (always > 2)
		  \return TRUE if to continue thread execution
		*/
		virtual bool onOverloadReceive(size_t ticksExpired)
		{
			return true;
		}
		//! On receive message from transport event handler
		/*!
		  Default implementation does nothing and returns true

		  \param msg Constant reference to the received message
		  \return True if to proceed with the message or false to discard it
		*/
		virtual bool onReceiveMessage(const MessageType& msg)
		{
			return true;
		}
		//! On provide incoming message to the consumer in the receiver thread event handler
		/*!
		  \param msg Constant reference to the provided message
		  \param consumer Reference to the message consumer where the message has been provided to
		*/
		virtual void onProvideMessage(const MessageType& msg, AbstractMessageConsumerType& consumer)
		{}
		//! After receiver execution event handler
		virtual void afterExecuteReceive()
		{}

		//! Before sender execution event handler
		virtual void beforeExecuteSend()
		{}
		//! On send message thread overload event handler
		/*!
		  Default implementation does nothing and returns TRUE.
		  \param Ticks expired (always > 2)
		  \return TRUE if to continue thread execution
		*/
		virtual bool onOverloadSend(size_t ticksExpired)
		{
			return true;
		}
		//! On consume message from any provider in sender thread event handler
		/*!
		  Default implementation does nothing and returns true

		  \param msg Constant reference to the consumed message
		  \return True if to proceed with the message or false to discard it
		*/
		virtual bool onConsumeMessage(const MessageType& msg)
		{
			return true;
		}
		//! On send message to transport event handler
		/*!
		  \param msg Constant reference to the message has been sent
		*/
		virtual void onSendMessage(const MessageType& msg)
		{}
		//! After sender execution event handler
		virtual void afterExecuteSend()
		{}

		//! Receiving message from transport abstract virtual method
		/*!
		  \param limit Data read time limit
		  \return Pointer to the received message or to 0 if no message have been received
		*/
		virtual MessageType * receiveMessage(const Timestamp& limit) = 0;
		//! Sending message to transport abstract method
		/*!
		  \param msg Constant reference to message to send
		  \param limit Data send time limit
		  \return True if the message has been sent
		*/
		virtual bool sendMessage(const MessageType& msg, const Timestamp& limit) = 0;
	private:
		//! Receive data task execution virtual method
		/*!
		  This method is to be executed in a separate worker thread.

		  \param taskDispatcher Reference to the task dispatcher subsystem
		*/
		virtual void executeReceiveImpl(MultiTaskDispatcher<AbstractAsyncTcpService::AbstractTask>& taskDispatcher)
		{
			isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Receiver thread execution has been started"));
			// Triggering before execute event
			beforeExecuteReceive();
			Ticker ticker(_service.clockTimeout());
			bool firstTick = true;
			while (true) {
				size_t ticksExpired;
				Timestamp nextTickLimit = ticker.tick(&ticksExpired);
				if (firstTick) {
					firstTick = false;
				} else if (ticksExpired > 1) {
					// Overload has been detected
					Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Receiver thread execution overload has been detected: ") << ticksExpired << " ticks expired");
					if (!onOverloadReceive(ticksExpired)) {
						isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS,
									"Receiver thread has been terminated by onOverloadReceive() event handler -> exiting from the thread execution"));
						appointTermination();
						break;
					}
				}
				// Reading messages until tick has been expired
				while (Timestamp::now() < nextTickLimit) {
					// Reading message from the transport
					std::auto_ptr<MessageType> msgAutoPtr;
					try {
						msgAutoPtr.reset(receiveMessage(nextTickLimit));
					} catch (Exception& e) {
						if (e.error().instanceOf<TcpSocket::ConnectionAbortedError>()) {
							// Terminating the task if the connection has been aborted
							isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Client connection has been aborted -> exiting from the receiver thread execution"));
							appointTermination();
							break;
						} else {
							throw;
						}
					}
					if (msgAutoPtr.get()) {
						isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Message has been received by the receiver thread execution"));
						// Calling on receive message event callback
						if (!onReceiveMessage(*msgAutoPtr.get())) {
							Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by the on receive event handler"));
							continue;
						}
						// Providing message to the internal output bus
						if (outputBus().push(*msgAutoPtr.get())) {
							onProvideMessage(*msgAutoPtr.get(), outputBus());
						}
						// Providing message to all consumers
						for (typename ConsumersContainer::iterator i = _service._consumers.begin(); i != _service._consumers.end(); ++i) {
							if ((*i)->push(*msgAutoPtr.get())) {
								onProvideMessage(*msgAutoPtr.get(), **i);
							}
						}
					}
				}
				// Checking termination
				if (shouldTerminate()) {
					isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Task termination has been detected -> exiting from the receiver thread execution"));
					break;
				}
				if (taskDispatcher.shouldTerminate()) {
					isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Task dispatcher termination has been detected -> exiting from the receiver thread execution"));
					break;
				}
			}
			// Triggering after execute event
			afterExecuteReceive();
		}
		//! Send data task execution virtual method
		/*!
		  This method is to be executed in a separate worker thread.

		  \param taskDispatcher Reference to the task dispatcher subsystem
		*/
		virtual void executeSendImpl(MultiTaskDispatcher<AbstractAsyncTcpService::AbstractTask>& taskDispatcher)
		{
			isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Sender thread execution has been started"));
			// Triggering before execute event
			beforeExecuteSend();
			std::auto_ptr<MessageType> currentMessageAutoPtr;
			bool sendingMessage = false;
			// Susbcribing input message queue to the providers
			typename MessageProviderType::SubscriberListReleaser subscriberListReleaser;
			for (typename ProvidersContainer::iterator i = _service._providers.begin(); i != _service._providers.end(); ++i) {
				std::auto_ptr<typename MessageProviderType::Subscriber> subscriberAutoPtr(new typename MessageProviderType::Subscriber(**i, inputQueue()));
				subscriberListReleaser.addSubscriber(subscriberAutoPtr.get());
				subscriberAutoPtr.release();
				Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Input message queue has been subscribed to the message provider"));
			}
			Ticker ticker(_service.clockTimeout());
			bool firstTick = true;
			while (true) {
				size_t ticksExpired;
				Timestamp nextTickLimit = ticker.tick(&ticksExpired);
				if (firstTick) {
					firstTick = false;
				} else if (ticksExpired > 1) {
					// Overload has been detected
					Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Sender thread execution overload has been detected: ") << ticksExpired << " ticks expired");
					if (!onOverloadSend(ticksExpired)) {
						isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS,
									"Sender thread has been terminated by onOverloadSend() event handler -> exiting from the thread execution"));
						appointTermination();
						break;
					}
				}
				// Consuming messages until tick has been expired
				while (Timestamp::now() < nextTickLimit) {
					if (sendingMessage) {
						// Sending message to peer
						bool messageSent = false;
						try {
							messageSent = sendMessage(*currentMessageAutoPtr.get(), nextTickLimit);
						} catch (Exception& e) {
							if (e.error().instanceOf<TcpSocket::ConnectionAbortedError>()) {
								// Terminating the task if the connection has been aborted
								isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Client connection has been aborted -> exiting from the sender thread execution"));
								appointTermination();
								break;
							} else {
								throw;
							}
						}
						if (messageSent) {
							isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Message has been sent by the sender thread execution"));
							sendingMessage = false;
							onSendMessage(*currentMessageAutoPtr.get());
						}
					} else if (_consumeBuffer.empty()) {
						// Fetching all messages from the input message queue to the consume buffer
						size_t consumedMessagesAmount = inputQueue().popAll(_consumeBuffer, nextTickLimit);
						if (consumedMessagesAmount > 0) {
							std::ostringstream oss;
							oss << consumedMessagesAmount << " message(s) has been fetched from the input queue to the consume buffer";
							Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
						}
					} else {
						// Fetching next message from the consume buffer
						currentMessageAutoPtr = _consumeBuffer.pop();
						if (onConsumeMessage(*currentMessageAutoPtr.get())) {
							sendingMessage = true;
						} else {
							Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by the on consume event handler"));
						}
					}
				}
				// Checking termination
				if (shouldTerminate()) {
					isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Task termination has been detected -> exiting from the sender thread execution"));
					break;
				}
				if (taskDispatcher.shouldTerminate()) {
					isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Task dispatcher termination has been detected -> exiting from the sender thread execution"));
					break;
				}
			}
			// Triggering after execute event
			afterExecuteSend();
		}

		AbstractMessageBrokerService& _service;
		StateSetType _stateSet;
		MessageBufferType _consumeBuffer;
		std::auto_ptr<MessageQueueType> _inputQueueAutoPtr;
		std::auto_ptr<MessageBusType> _outputBusAutoPtr;
	};

	//! Input message queue creation factory method
	/*!
	  \return Auto-pointer to the input queue object
	*/
	virtual MessageQueueType * createInputQueue(AbstractTask& task)
	{
		return new MessageQueueType();
	}
	//! Output message bus creation factory method
	/*!
	  \return Auto-pointer to the output bus object
	*/
	virtual MessageBusType * createOutputBus(AbstractTask& task)
	{
		return new MessageBusType();
	}
private:
	AbstractMessageBrokerService();
	AbstractMessageBrokerService(const AbstractMessageBrokerService&);						// No copy

	AbstractMessageBrokerService& operator=(const AbstractMessageBrokerService&);					// No copy

	typedef std::list<MessageProviderType *> ProvidersContainer;
	typedef std::list<AbstractMessageConsumerType *> ConsumersContainer;

	ProvidersContainer _providers;
	ConsumersContainer _consumers;
};

} // namespace isl

#endif

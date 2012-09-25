#ifndef ISL__ABSTRACT_MESSAGE_BROKER_SERVICE__HXX
#define ISL__ABSTRACT_MESSAGE_BROKER_SERVICE__HXX

#include <isl/common.hxx>
#include <isl/AbstractAsyncTcpService.hxx>
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
  to execute two tasks per each client connection: message sender and receiver ones. These tasks should be
  objects of  the descendants of the following abstract classes:

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

  \tparam Msg Message class
  \tparam Cloner Message cloner class with static <tt>Msg * Cloner::clone(const Msg& msg)</tt> method for cloning the message
*/
template <typename Msg, typename Cloner = CopyMessageCloner<Msg> > class AbstractMessageBrokerService : public AbstractAsyncTcpService
{
public:
	typedef Msg MessageType;						//!< Message type
	typedef MessageProvider<Msg> MessageProviderType;			//!< Message provider type
	typedef AbstractMessageConsumer<Msg> AbstractMessageConsumerType;	//!< Abstract message consumer type
	typedef MessageQueue<Msg, Cloner> MessageQueueType;			//!< Message queue type
	typedef MessageBuffer<Msg, Cloner> MessageBufferType;			//!< Message buffer type
	typedef MessageBus<Msg> MessageBusType;					//!< Message bus type

	//! Constructor
	/*!
	  \param owner Pointer to the owner subsystem
	  \param maxClients Maximum clients amount
	  \param clockTimeout Subsystem's clock timeout
	  \param maxTaskQueueOverflowSize Maximum tasks queue overflow size
	*/
	AbstractMessageBrokerService(Subsystem * owner, size_t maxClients, const Timeout& clockTimeout = Timeout::defaultTimeout(),
			size_t maxTaskQueueOverflowSize = 0) :
		AbstractAsyncTcpService(owner, maxClients, maxTaskQueueOverflowSize),
		_clockTimeout(clockTimeout),
		_providers(),
		_consumers()
	{}
	//! Returns clock timeout
	inline Timeout clockTimeout() const
	{
		ReadLocker locker(runtimeParamsRWLock);
		return _clockTimeout;
	}
	//! Sets clock timeout
	/*!
	  Subsystem's restart needed to activate changes.

	  \param newValue New clock timeout
	*/
	inline void setClockTimeout(const Timeout& newValue)
	{
		WriteLocker locker(runtimeParamsRWLock);
		_clockTimeout = newValue;
	}
	//! Adds message provider to subscribe input queue to while running
	/*!
	  Subsystem's restart needed to activate changes.

	  \param provider Reference to provider to add
	*/
	void addProvider(MessageProviderType& provider)
	{
		WriteLocker locker(runtimeParamsRWLock);
		_providers.push_back(&provider);
	}
	//! Removes message provider
	/*!
	  Subsystem's restart needed to activate changes.

	  \param provider Reference to provider to remove
	*/
	void removeProvider(MessageProviderType& provider)
	{
		WriteLocker locker(runtimeParamsRWLock);
		typename ProvidersContainer::iterator pos = std::find(_providers.begin(), _providers.end(), &provider);
		if (pos == _providers.end()) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message provider not found"));
			return;
		}
		_providers.erase(pos);
	}
	//! Removes all message providers
	/*!
	  Subsystem's restart needed to activate changes.
	*/
	inline void resetProviders()
	{
		WriteLocker locker(runtimeParamsRWLock);
		_providers.clear();
	}
	//! Adds message consumer for providing incoming messages to while running
	/*!
	  Subsystem's restart needed to activate changes.

	  \param consumer Reference to consumer to add
	*/
	void addConsumer(AbstractMessageConsumerType& consumer)
	{
		WriteLocker locker(runtimeParamsRWLock);
		_consumers.push_back(&consumer);
	}
	//! Removes message consumer
	/*!
	  Subsystem's restart needed to activate changes.

	  \param consumer Reference to consumer to remove
	*/
	void removeConsumer(AbstractMessageConsumerType& consumer)
	{
		WriteLocker locker(runtimeParamsRWLock);
		typename ConsumersContainer::iterator pos = std::find(_consumers.begin(), _consumers.end(), &consumer);
		if (pos == _consumers.end()) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message consumer not found"));
			return;
		}
		_consumers.erase(pos);
	}
	//! Removes all message consumers
	/*!
	  Subsystem's restart needed to activate changes.
	*/
	void resetConsumers()
	{
		WriteLocker locker(runtimeParamsRWLock);
		_consumers.clear();
	}
protected:
	//! Shared staff which is to be used by sender and receiver tasks concurrently
	class SharedStaff : public AbstractAsyncTcpService::SharedStaff
	{
	public:
		//! Constructor
		/*!
		  \param socket Reference to the client connection socket
		*/
		SharedStaff(TcpSocket& socket) :
			AbstractAsyncTcpService::SharedStaff(socket),
			_inputQueueAutoPtr(),
			_outputBusAutoPtr()
		{}
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
		//! Sends a request message to message broker client and waits for response(-s)
		/*!
		  \param request Constant reference to request message to send
		  \param responseQueue Reference to response-filtering message queue to save a response(-s) to
		  \param timeout Timeout to wait for response
		  \return True if the message has been accepted by input message queue
		*/
		inline bool sendRequest(const Msg& request, MessageQueueType& responseQueue, const Timeout& timeout = Timeout::defaultTimeout())
		{
			responseQueue.clear();
			typename MessageProviderType::Subscriber subscriber(outputBus(), responseQueue);
			if (!inputQueue().push(request)) {
				return false;
			}
			return responseQueue.await(timeout);
		}
	protected:
		//! Shared staff initialization virtual method overriding
		virtual void init()
		{
			_inputQueueAutoPtr.reset(createInputQueue());
			_outputBusAutoPtr.reset(createOutputBus());
		}
		//! Input message queue creation factory method
		/*!
		  \return Auto-pointer to the input queue object
		*/
		virtual MessageQueueType * createInputQueue()
		{
			return new MessageQueueType();
		}
		//! Output message bus creation factory method
		/*!
		  \return Auto-pointer to the output bus object
		*/
		virtual MessageBusType * createOutputBus()
		{
			return new MessageBusType();
		}
	private:
		std::auto_ptr<MessageQueueType> _inputQueueAutoPtr;
		std::auto_ptr<MessageBusType> _outputBusAutoPtr;

		friend class AbstractReceiverTask;
		friend class AbstractSenderTask;
	};
	//! Receiver task object abstract class
	class AbstractReceiverTask : public AbstractAsyncTcpService::AbstractReceiverTask
	{
	public:
		//! Constructor
		/*!
		  \param service Reference to message broker service
		  \param sharedStaff Reference to the shared staff object
		*/
		AbstractReceiverTask(AbstractMessageBrokerService& service, SharedStaff& sharedStaff) :
			AbstractAsyncTcpService::AbstractReceiverTask(service, sharedStaff),
			_service(service),
			_sharedStaff(sharedStaff)
		{}
		//! Returns a reference to the input message queue
		inline MessageQueueType& inputQueue()
		{
			return _sharedStaff.inputQueue();
		}
	protected:
		//! Before execute event handler
		virtual void beforeExecute()
		{}
		//! On receive message from transport event handler
		/*!
		  Default implementation does nothing and returns true

		  \param msg Constant reference to the received message
		  \return True if to proceed with the message or false to discard it
		*/
		virtual bool onReceiveMessage(const Msg& msg)
		{
			return true;
		}
		//! On provide incoming message to the consumer event handler
		/*!
		  \param msg Constant reference to the provided message
		  \param consumer Reference to the message consumer where the message has been provided to
		*/
		virtual void onProvideMessage(const Msg& msg, AbstractMessageConsumerType& consumer)
		{}
		//! On receive data from transport exception event handler
		/*!
		  \param e Pointer to std::exception instance or 0 if exception is unknown
		*/
		virtual void onReceiveDataException(std::exception * e = 0)
		{}
		//! After execute event handler
		virtual void afterExecute()
		{}
		//! Receiving message from transport abstract method
		/*!
		  \param timeout Data read timeout
		  \return Auto-pointer to the received message or to 0 if no message have been received
		*/
		virtual std::auto_ptr<Msg> receiveMessage(const Timeout& timeout) = 0;
	private:
		virtual void execute(TaskDispatcherType::WorkerThread& worker)
		{
			isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Receiver task execution has been started"));
			// Triggering before execute event
			beforeExecute();
			// Fetching runtime parameters
			Timeout clockTimeout;
			ConsumersContainer consumers;
			{
				ReadLocker locker(_service.runtimeParamsRWLock);
				clockTimeout = _service._clockTimeout;
				consumers = _service._consumers;
			}
			while (true) {
				if (_sharedStaff.shouldTerminate()) {
					isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Shared staff termination has been detected -> exiting from receiver task execution"));
					break;
				}
				if (worker.shouldTerminate()) {
					isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Worker termination has been detected -> exiting from receiver task execution"));
					break;
				}
				// Reading message from the device
				std::auto_ptr<Msg> msgAutoPtr;
				try {
					msgAutoPtr = receiveMessage(clockTimeout);
				} catch (std::exception& e) {
					onReceiveDataException(&e);
					_sharedStaff.setShouldTerminate(true);
					break;
				} catch (...) {
					onReceiveDataException();
					_sharedStaff.setShouldTerminate(true);
					break;
				}
				if (msgAutoPtr.get()) {
					// Calling on receive message event callback
					if (!onReceiveMessage(*msgAutoPtr.get())) {
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by the on receive event handler"));
						continue;
					}
					// Providing message to the internal output bus
					if (_sharedStaff.outputBus().push(*msgAutoPtr.get())) {
						onProvideMessage(*msgAutoPtr.get(), _sharedStaff.outputBus());
					}
					// Providing message to all consumers
					for (typename ConsumersContainer::iterator i = consumers.begin(); i != consumers.end(); ++i) {
						if ((*i)->push(*msgAutoPtr.get())) {
							onProvideMessage(*msgAutoPtr.get(), **i);
						}
					}
				}
			}
			// Triggering after execute event
			afterExecute();
		}

		AbstractMessageBrokerService& _service;
		SharedStaff& _sharedStaff;
	};
	//! Sender task object abstract class
	class AbstractSenderTask : public AbstractAsyncTcpService::AbstractSenderTask
	{
	public:
		//! Constructor
		/*!
		  \param service Reference to message broker service
		  \param sharedStaff Reference to the shared staff object
		*/
		AbstractSenderTask(AbstractMessageBrokerService& service, SharedStaff& sharedStaff) :
			AbstractAsyncTcpService::AbstractSenderTask(service, sharedStaff),
			_service(service),
			_sharedStaff(sharedStaff),
			_consumeBuffer()
		{}

		//! Returns a reference to the input message queue
		inline MessageQueueType& inputQueue()
		{
			return _sharedStaff.inputQueue();
		}
	protected:
		//! Before execute event handler
		virtual void beforeExecute()
		{}
		//! On consume message from any provider event handler
		/*!
		  Default implementation does nothing and returns true

		  \param msg Constant reference to the consumed message
		  \return True if to proceed with the message or false to discard it
		*/
		virtual bool onConsumeMessage(const Msg& msg)
		{
			return true;
		}
		//! On send message to transport event handler
		/*!
		  \param msg Constant reference to the message has been sent
		*/
		virtual void onSendMessage(const Msg& msg)
		{}
		//! On send data to transport exception event handler
		/*!
		  \param e Pointer to std::exception instance or 0 if exception is unknown
		*/
		virtual void onSendDataException(std::exception * e = 0)
		{}
		//! After execute event handler
		virtual void afterExecute()
		{}
		//! Sending message to transport abstract method
		/*!
		  \param msg Constant reference to message to send
		  \param timeout Data send timeout
		  \return True if the message has been sent
		*/
		virtual bool sendMessage(const Msg& msg, const Timeout& timeout) = 0;
	private:
		virtual void execute(TaskDispatcherType::WorkerThread& worker)
		{
			isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Sender task execution has been started"));
			// Triggering before execute event
			beforeExecute();
			std::auto_ptr<Msg> currentMessageAutoPtr;
			bool sendingMessage = false;
			// Fetching runtime parameters
			Timeout clockTimeout;
			ProvidersContainer providers;
			{
				ReadLocker locker(_service.runtimeParamsRWLock);
				clockTimeout = _service._clockTimeout;
				providers = _service._providers;
			}
			// Susbcribing input message queue to the providers
			typename MessageProviderType::SubscriberListReleaser subscriberListReleaser;
			for (typename ProvidersContainer::iterator i = providers.begin(); i != providers.end(); ++i) {
				std::auto_ptr<typename MessageProviderType::Subscriber> subscriberAutoPtr(new typename MessageProviderType::Subscriber(**i, inputQueue()));
				subscriberListReleaser.addSubscriber(subscriberAutoPtr.get());
				subscriberAutoPtr.release();
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Sender task's input queue has been subscribed to the message provider"));
			}
			while (true) {
				if (_sharedStaff.shouldTerminate()) {
					isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Shared staff termination has been detected -> exiting from receiver task execution"));
					break;
				}
				if (worker.shouldTerminate()) {
					isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Worker termination has been detected -> exiting from receiver task execution"));
					break;
				}
				if (sendingMessage) {
					// Sending message to peer
					bool messageSent = false;
					try {
						messageSent = sendMessage(*currentMessageAutoPtr.get(), clockTimeout);
					} catch (std::exception& e) {
						onSendDataException(&e);
						_sharedStaff.setShouldTerminate(true);
						break;
					} catch (...) {
						_sharedStaff.setShouldTerminate(true);
						onSendDataException();
						break;
					}
					if (messageSent) {
						sendingMessage = false;
						onSendMessage(*currentMessageAutoPtr.get());
					}
				} else if (_consumeBuffer.empty()) {
					// Fetching all messages from the input to the consume buffer
					size_t consumedMessagesAmount = inputQueue().popAll(_consumeBuffer, clockTimeout);
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
			// Triggering after execute event
			afterExecute();
		}

		AbstractMessageBrokerService& _service;
		SharedStaff& _sharedStaff;
		MessageBufferType _consumeBuffer;
	};

	//! Shared staff creation factory method
	/*!
	  \param socket Reference to the client connection socket
	  \return Pointer to the new shared staff object
	*/
	virtual AbstractAsyncTcpService::SharedStaff * createSharedStaff(TcpSocket& socket)
	{
		return new SharedStaff(socket);
	}
private:
	AbstractMessageBrokerService();
	AbstractMessageBrokerService(const AbstractMessageBrokerService&);						// No copy

	AbstractMessageBrokerService& operator=(const AbstractMessageBrokerService&);					// No copy

	typedef std::list<MessageProviderType *> ProvidersContainer;
	typedef std::list<AbstractMessageConsumerType *> ConsumersContainer;

	const Timeout _clockTimeout;
	ProvidersContainer _providers;
	ConsumersContainer _consumers;
};

} // namespace isl

#endif

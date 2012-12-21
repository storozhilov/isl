#ifndef ISL__ABSTRACT_MESSAGE_BROKER_CONNECTION__HXX
#define ISL__ABSTRACT_MESSAGE_BROKER_CONNECTION__HXX

#include <isl/common.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/TcpAddrInfo.hxx>
#include <isl/Subsystem.hxx>
#include <isl/IOError.hxx>
#include <isl/MessageQueue.hxx>
#include <isl/MessageBuffer.hxx>
#include <isl/MessageProvider.hxx>
#include <isl/MessageBus.hxx>
#include <isl/Error.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>
#include <isl/MemFunThread.hxx>
#include <memory>

namespace isl
{

//! Message broker connection subsystem abstract templated class
/*!
  Use this class for your message broker connection implementation. It creates two threads:
  one is for receiving messages and another is for sending ones. So, you should define
  following two abstract virtual methods in your subclass:

  - AbstractMessageBrokerConnection::receiveMessage() - receives message from the transport;
  - AbstractMessageBrokerConnection::sendMessage() - sends message to the transport.

  TCP-connection control is provided by message receiver thread, which is automatically re-establishes it if aborted.
  A thrown Exception with TcpSocket::ConnectionAbortedError error from the receiveMessage()/sendMessage()
  method is used as signal for reopening TCP-connection socket.
  
  \tparam Msg Message class
  \tparam Cloner Message cloner class with static <tt>Msg * Cloner::clone(const Msg& msg)</tt> method for cloning the message
*/
template <typename Msg, typename Cloner = CopyMessageCloner<Msg> > class AbstractMessageBrokerConnection : public Subsystem
{
public:
	typedef Msg MessageType;							//!< Message type
	typedef MessageProvider<MessageType> MessageProviderType;			//!< Message provider type
	typedef AbstractMessageConsumer<MessageType> AbstractMessageConsumerType;	//!< Abstract message consumer type
	typedef MessageQueue<MessageType, Cloner> MessageQueueType;			//!< Message queue type
	typedef MessageBuffer<MessageType, Cloner> MessageBufferType;			//!< Message buffer type
	typedef MessageBus<MessageType> MessageBusType;					//!< Message bus type

	//! Input message queue factory base class
	class InputQueueFactory
	{
	public:
		virtual ~InputQueueFactory()
		{}
		//! Input message queue creation factory method
		virtual MessageQueueType * create() const
		{
			return new MessageQueueType();
		}
	};

	//! Output message bus factory base class
	class OutputBusFactory
	{
	public:
		virtual ~OutputBusFactory()
		{}
		//! Output message bus creation factory method
		virtual MessageBusType * create() const
		{
			return new MessageBusType();
		}
	};

	//! Constructor
	/*!
	  \param owner Pointer to the owner subsystem
	  \param remoteAddr Message broker remote address info
	  \param clockTimeout Subsystem's clock timeout
	  \param inputQueueFactory Input message queue factory object reference
	  \param outputBusFactory Output message bus factory object reference
	*/
	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& remoteAddr,
			const Timeout& clockTimeout = Timeout::defaultTimeout(), const InputQueueFactory& inputQueueFactory = InputQueueFactory(),
			const OutputBusFactory& outputBusFactory = OutputBusFactory()) :
		Subsystem(owner),
		_remoteAddr(remoteAddr),
		_inputQueueAutoPtr(inputQueueFactory.create()),
		_providedInputQueuePtr(),
		_outputBusAutoPtr(outputBusFactory.create()),
		_providedOutputBusPtr(),
		_clockTimeout(clockTimeout),
		_receiverRequester(),
		_senderRequester(),
		_receiverThread(),
		_senderThread(),
		_socket(),
		_consumeBuffer(),
		_providers(),
		_consumers()
	{}
	//! Constructor with user provided input message queue
	/*!
	  \param owner Pointer to the owner subsystem
	  \param remoteAddr Message broker remote address info
	  \param inputQueue Reference to the input message queue to fetch messages from
	  \param clockTimeout Subsystem's clock timeout
	  \param outputBusFactory Output message bus factory object reference
	*/
	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& remoteAddr, MessageQueueType& inputQueue,
			const Timeout& clockTimeout = Timeout::defaultTimeout(), const OutputBusFactory& outputBusFactory = OutputBusFactory()) :
		Subsystem(owner),
		_remoteAddr(remoteAddr),
		_inputQueueAutoPtr(),
		_providedInputQueuePtr(&inputQueue),
		_outputBusAutoPtr(outputBusFactory.create()),
		_providedOutputBusPtr(),
		_clockTimeout(clockTimeout),
		_receiverRequester(),
		_senderRequester(),
		_receiverThread(),
		_senderThread(),
		_socket(),
		_consumeBuffer(),
		_providers(),
		_consumers()
	{}
	//! Constructor with user provided output message bus
	/*!
	  \param owner Pointer to the owner subsystem
	  \param remoteAddr Message broker remote address info
	  \param outputBus Reference to the output message bus to provide all received messages to
	  \param clockTimeout Subsystem's clock timeout
	  \param inputQueueFactory Input message queue factory object reference
	*/
	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& remoteAddr, MessageBusType& outputBus,
			const Timeout& clockTimeout = Timeout::defaultTimeout(), const InputQueueFactory& inputQueueFactory = InputQueueFactory()) :
		Subsystem(owner),
		_remoteAddr(remoteAddr),
		_inputQueueAutoPtr(inputQueueFactory.create()),
		_providedInputQueuePtr(),
		_outputBusAutoPtr(),
		_providedOutputBusPtr(&outputBus),
		_clockTimeout(clockTimeout),
		_receiverRequester(),
		_senderRequester(),
		_receiverThread(),
		_senderThread(),
		_socket(),
		_consumeBuffer(),
		_providers(),
		_consumers()
	{}
	//! Constructor with user provided input message queue and output message bus
	/*!
	  \param owner Pointer to the owner subsystem
	  \param remoteAddr Message broker remote address info
	  \param inputQueue Reference to the input message queue to fetch messages from
	  \param outputBus Reference to the output message bus to provide all received messages to
	  \param clockTimeout Subsystem's clock timeout
	*/
	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& remoteAddr, MessageQueueType& inputQueue,
			MessageBusType& outputBus, const Timeout& clockTimeout = Timeout::defaultTimeout()) :
		Subsystem(owner),
		_remoteAddr(remoteAddr),
		_inputQueueAutoPtr(),
		_providedInputQueuePtr(&inputQueue),
		_outputBusAutoPtr(),
		_providedOutputBusPtr(&outputBus),
		_clockTimeout(clockTimeout),
		_receiverRequester(),
		_senderRequester(),
		_receiverThread(),
		_senderThread(),
		_socket(),
		_consumeBuffer(),
		_providers(),
		_consumers()
	{}
	//! Returns a reference to the input message queue
	inline MessageQueueType& inputQueue()
	{
		if (_providedInputQueuePtr) {
			return *_providedInputQueuePtr;
		} else if (_inputQueueAutoPtr.get()) {
			return *_inputQueueAutoPtr.get();
		} else {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Input message queue have not been initialized"));
		}
	}
	//! Returns a reference to the output message bus
	inline MessageBusType& outputBus()
	{
		if (_providedOutputBusPtr) {
			return *_providedOutputBusPtr;
		} else if (_outputBusAutoPtr.get()) {
			return *_outputBusAutoPtr.get();
		} else {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Output message bus have not been initialized"));
		}
	}
	//! Returns message broker remote address
	TcpAddrInfo remoteAddr() const
	{
		return _remoteAddr;
	}
	//! Sets message broker remote address
	/*!
	  \param newValue New message broker address

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	void setRemoteAddr(const TcpAddrInfo& newValue)
	{
		_remoteAddr = newValue;
	}
	//! Adds message provider to subscribe input queue to while running
	/*!
	  \param provider Reference to provider to add

	  \note Thread-unsafe: call it when subsystem is idling only
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
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message provider not found in connection"));
			return;
		}
		_providers.erase(pos);
	}
	//! Removes all message providers
	/*!
	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	void resetProviders()
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
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message consumer not found in connection"));
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
	//! Returns clock timeout
	Timeout clockTimeout() const
	{
		return _clockTimeout;
	}
	//! Sets clock timeout
	/*!
	  \param newValue New clock timeout

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	void setClockTimeout(const Timeout& newValue)
	{
		_clockTimeout = newValue;
	}
	//! Sends a message to message broker
	inline bool sendMessage(const MessageType& msg)
	{
		return inputQueue().push(msg);
	}
	//! Sends a request message to message broker and waits for response(-s)
	/*!
	  \param request Constant reference to request message to send
	  \param responseQueue Reference to response-filtering message queue to save a response(-s) to
	  \param timeout Timeout to wait for response
	  \return True if the message has been accepted by input message queue
	*/
	inline bool sendRequest(const MessageType& request, MessageQueueType& responseQueue, const Timeout& timeout = Timeout::defaultTimeout())
	{
		responseQueue.clear();
		typename MessageProviderType::Subscriber subscriber(outputBus(), responseQueue);
		if (!inputQueue().push(request)) {
			return false;
		}
		return responseQueue.await(timeout);
	}
	//! Starting subsystem method redefinition
	virtual void start()
	{
		// Calling ancestor's method
		Subsystem::start();
		// Starting receiver and sender threads
		_senderRequester.reset();
		_receiverRequester.reset();
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Starting receiver thread"));
		_receiverThread.start(*this, &AbstractMessageBrokerConnection<Msg, Cloner>::receive);
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Starting sender thread"));
		_senderThread.start(*this, &AbstractMessageBrokerConnection<Msg, Cloner>::send);
	}
	//! Stopting subsystem method redefinition
	virtual void stop()
	{
		// Sending termination requests to the sender and receiver threads
		size_t senderRequestId = _senderRequester.sendRequest(TerminateRequestMessage());
		if (senderRequestId > 0) {
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Termination request has been sent to the sender thread"));
		} else {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Could not send termination request to the sender thread"));
			// TODO _senderThread.kill();
		}
		size_t receiverRequestId = _receiverRequester.sendRequest(TerminateRequestMessage());
		if (receiverRequestId > 0) {
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Termination request has been sent to the receiver thread"));
		} else {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Could not send termination request to the receiver thread"));
			// TODO _receiverThread.kill();
		}
		// Awaiting for sender thread termination
		if (senderRequestId > 0) {
			std::auto_ptr<AbstractInterThreadMessage> responseAutoPtr = _senderRequester.awaitResponse(senderRequestId, awaitResponseTimeout());
			if (!responseAutoPtr.get()) {
				std::ostringstream msg;
				msg << "No response to termination request have been received from the sender thread";
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			} else if (responseAutoPtr->instanceOf<OkResponseMessage>()) {
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "OK response to the termination request has been received from the sender thread"));
			} else {
				std::ostringstream msg;
				msg << "Invalid response to termination request has been received from the sender thread: \"" << responseAutoPtr->name() << "\"";
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Joining a sender thread"));
			_senderThread.join();
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Sender thread has been terminated"));
		}
		// Awaiting for receiver thread termination
		if (receiverRequestId > 0) {
			std::auto_ptr<AbstractInterThreadMessage> responseAutoPtr = _receiverRequester.awaitResponse(receiverRequestId, awaitResponseTimeout());
			if (!responseAutoPtr.get()) {
				std::ostringstream msg;
				msg << "No response to termination request have been received from the receiver thread";
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			} else if (responseAutoPtr->instanceOf<OkResponseMessage>()) {
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "OK response to the termination request has been received from the receiver thread"));
			} else {
				std::ostringstream msg;
				msg << "Invalid response to termination request has been received from the receiver thread: \"" << responseAutoPtr->name() << "\"";
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Joining a receiver thread"));
			_receiverThread.join();
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Receiver thread has been terminated"));
		}
		// Calling ancestor's method
		Subsystem::stop();
	}
protected:
	//! On connected event handler which is to be called in message receiver thread
	/*!
	  \param socket Reference to the connected socket
	*/
	virtual void onReceiverConnected(TcpSocket& socket)
	{}
	//! On disconnected event handler which is to be called in message receiver thread
	/*!
	  \param isConnectionAborted TRUE if the connection has been aborted
	*/
	virtual void onReceiverDisconnected(bool isConnectionAborted)
	{}
	//! On connect exception event handler which is to be called in message receiver thread
	/*!
	  \param e Constant reference to the exception object
	  \param connectionAttempts Current unsuccessful connection attempts amount
	*/
	void onConnectException(const Exception& e, size_t connectionAttempts)
	{}
	//! On receive message from transport event handler
	/*!
	  \note Default implementation does nothing and returns TRUE
	  \param msg Constant reference to the received message
	  \return TRUE if to proceed with the message or FALSE to discard it
	*/
	virtual bool onReceiveMessage(const MessageType& msg)
	{
		return true;
	}
	//! On provide incoming message to the consumer event handler
	/*!
	  \param msg Constant reference to the provided message
	  \param consumer Reference to the message consumer where the message has been provided to
	*/
	virtual void onProvideMessage(const MessageType& msg, AbstractMessageConsumerType& consumer)
	{}
	//! On unrecognized inter-thread request to receiver event handler
	/*!
	  \note Default implementation does nothing and returns FALSE
	  \param request Constant reference to the inter-thread request
	  \return TRUE if the request has been successfully recognized and handled
	*/
	virtual bool onReceiverRequest(const AbstractInterThreadMessage& request)
	{
		return false;
	}
	//! On connected event handler which is to be called in message sender thread
	/*!
	  \param socket Reference to the connected socket
	*/
	virtual void onSenderConnected(TcpSocket& socket)
	{}
	//! On disconnected event handler which is to be called in message sender thread
	/*!
	  \param isConnectionAborted TRUE if the connection has been aborted
	*/
	virtual void onSenderDisconnected(bool isConnectionAborted)
	{}
	//! On consume message from any provider event handler
	/*!
	  \note Default implementation does nothing and returns TRUE
	  \param msg Constant reference to the consumed message
	  \return TRUE if to proceed with the message or FALSE to discard it
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
	//! On unrecognized inter-thread request to sender event handler
	/*!
	  \note Default implementation does nothing and returns FALSE
	  \param request Constant reference to the inter-thread request
	  \return TRUE if the request has been successfully recognized and handled
	*/
	virtual bool onSenderRequest(const AbstractInterThreadMessage& request)
	{
		return false;
	}

	//! Receiving message from transport abstract method
	/*!
	  \param socket Socket to read data from
	  \param timeout Data read timeout
	  \return Pointer to the received message or to 0 if no message has been received
	*/
	virtual MessageType * receiveMessage(TcpSocket& socket, const Timeout& timeout) = 0;
	//! Sending message to transport abstract method
	/*!
	  \param msg Constant reference to message to send
	  \param socket Socket to send data to
	  \param timeout Data send timeout
	  \return True if the message has been sent
	*/
	virtual bool sendMessage(const MessageType& msg, TcpSocket& socket, const Timeout& timeout) = 0;
private:
	AbstractMessageBrokerConnection();
	AbstractMessageBrokerConnection(const AbstractMessageBrokerConnection&);						// No copy

	AbstractMessageBrokerConnection& operator=(const AbstractMessageBrokerConnection&);					// No copy

	typedef MemFunThread<AbstractMessageBrokerConnection<Msg, Cloner> > TransmitterThread;
	typedef std::list<MessageProviderType *> ProvidersContainer;
	typedef std::list<AbstractMessageConsumerType *> ConsumersContainer;

	class ConnectRequestMessage : public AbstractInterThreadMessage
	{
	public:
		virtual const char * name() const
		{
			static const char * n = "Connect Request";
			return n;
		}
		virtual AbstractInterThreadMessage * clone() const
		{
			return new ConnectRequestMessage(*this);
		}
	};

	class DisconnectRequestMessage : public AbstractInterThreadMessage
	{
	public:
		virtual const char * name() const
		{
			static const char * n = "Disconnect Request";
			return n;
		}
		virtual AbstractInterThreadMessage * clone() const
		{
			return new DisconnectRequestMessage(*this);
		}
	};

	void receive()
	{
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Receiver thread has been started"));
		_socket.open();
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Socket has been opened"));
		bool connected = false;
		size_t connectionAttempts = 0;
		while (true) {
			// Handling incoming inter-thread request
			const InterThreadRequesterType::PendingRequest * pendingRequestPtr = 0;
			if (connected) {
				pendingRequestPtr = _receiverRequester.fetchRequest();
			} else {
				pendingRequestPtr = _receiverRequester.awaitRequest(clockTimeout());
			}
			if (pendingRequestPtr) {
				if (pendingRequestPtr->request().instanceOf<TerminateRequestMessage>()) {
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Termination request has been received by the receiver thread -> exiting from the receiver thread"));
					if (pendingRequestPtr->responseRequired()) {
						_receiverRequester.sendResponse(OkResponseMessage());
					}
					break;
				} else if (!onReceiverRequest(pendingRequestPtr->request())) {
					std::ostringstream msg;
					msg << "Unknown inter-thread request has been received by the receiver thread: \"" << pendingRequestPtr->request().name() << '"';
					warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
				}
			}
			if (connected) {
				// Receiving message if connected
				std::auto_ptr<MessageType> msgAutoPtr;
				try {
					msgAutoPtr.reset(receiveMessage(_socket, _clockTimeout));
				} catch (Exception& e) {
					if (e.error().instanceOf<TcpSocket::ConnectionAbortedError>()) {
						connected = false;
					} else {
						throw;
					}
				}
				if (!connected) {
					isl::errorLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection has been aborted in the receiver thread"));
					connectionAttempts = 0;
					// Sending disconnect request to the sender thread
					size_t requestId = _senderRequester.sendRequest(DisconnectRequestMessage());
					if (requestId > 0) {
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Disconnect request has been sent to the sender thread"));
					} else {
						errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Could not send disconnect request to the sender thread"));
					}
					// Calling 'on receiver disconnected' event handler
					onReceiverDisconnected(true);
					// Awaiting for the response from the sender thread
					if (requestId > 0) {
						std::auto_ptr<AbstractInterThreadMessage> responseAutoPtr = _senderRequester.awaitResponse(requestId, awaitResponseTimeout());
						if (!responseAutoPtr.get()) {
							std::ostringstream msg;
							msg << "No response to disconnect request have been received from the sender thread";
							errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
						} else if (responseAutoPtr->instanceOf<OkResponseMessage>()) {
							debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "OK response to the disconnect request has been received from the sender thread"));
						} else {
							std::ostringstream msg;
							msg << "Invalid response to disconnect request has been received from the sender thread: \"" << responseAutoPtr->name() << "\"";
							errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
						}
					}
					// Reopening socket
					_socket.close();
					_socket.open();
				} else if (msgAutoPtr.get()) {
					isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Message has been received by the receiver thread execution"));
					// Calling on receive message event callback
					if (!onReceiveMessage(*msgAutoPtr.get())) {
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by the on receive event handler"));
						continue;
					}
					// Providing message to the internal output message bus
					if (outputBus().push(*msgAutoPtr.get())) {
						onProvideMessage(*msgAutoPtr.get(), outputBus());
					}
					// Providing message to all consumers
					for (typename ConsumersContainer::iterator i = _consumers.begin(); i != _consumers.end(); ++i) {
						if ((*i)->push(*msgAutoPtr.get())) {
							onProvideMessage(*msgAutoPtr.get(), **i);
						}
					}
				}
			} else {
				// Establishing connection if not connected
				try {
					_socket.connect(_remoteAddr);
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection has been established"));
					connected = true;
					// Sending connect request to the sender thread
					size_t requestId = _senderRequester.sendRequest(ConnectRequestMessage());
					if (requestId > 0) {
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Connect request has been sent to the sender thread"));
					} else {
						errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Could not send connect request to the sender thread"));
					}
					// Calling 'on receiver connected' event handler
					onReceiverConnected(_socket);
					// Awaiting for the response from the sender thread
					if (requestId > 0) {
						std::auto_ptr<AbstractInterThreadMessage> responseAutoPtr = _senderRequester.awaitResponse(requestId, awaitResponseTimeout());
						if (!responseAutoPtr.get()) {
							std::ostringstream msg;
							msg << "No response to connect request have been received from the sender thread";
							errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
						} else if (responseAutoPtr->instanceOf<OkResponseMessage>()) {
							debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "OK response to the connect request has been received from the sender thread"));
						} else {
							std::ostringstream msg;
							msg << "Invalid response to connect request has been received from the sender thread: \"" << responseAutoPtr->name() << "\"";
							errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
						}
					}
				} catch (Exception& e) {
					++connectionAttempts;
					if (connectionAttempts % 10 == 0) {
						errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Establishing message broker connection error"));
					}
					onConnectException(e, connectionAttempts);
				}
			}
		}
		if (connected) {
			_socket.close();
			isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection has been closed"));
			onReceiverDisconnected(false);
		}
	}

	void send()
	{
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Sender thread has been started"));
		std::auto_ptr<MessageType> currentMessageAutoPtr;
		bool sendingMessage = false;
		bool connected = false;
		// Susbcribing input message queue to the providers
		typename MessageProviderType::SubscriberListReleaser subscriberListReleaser;
		for (typename ProvidersContainer::iterator i = _providers.begin(); i != _providers.end(); ++i) {
			std::auto_ptr<typename MessageProviderType::Subscriber> subscriberAutoPtr(new typename MessageProviderType::Subscriber(**i, inputQueue()));
			subscriberListReleaser.addSubscriber(subscriberAutoPtr.get());
			subscriberAutoPtr.release();
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Input queue has been subscribed to the message provider"));
		}
		while (true) {
			// Handling incoming inter-thread request
			const InterThreadRequesterType::PendingRequest * pendingRequestPtr = 0;
			if (connected) {
				pendingRequestPtr = _senderRequester.fetchRequest();
			} else {
				pendingRequestPtr = _senderRequester.awaitRequest(clockTimeout());
			}
			if (pendingRequestPtr) {
				if (pendingRequestPtr->request().instanceOf<TerminateRequestMessage>()) {
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Termination request has been received by the sender thread -> exiting from the sender thread"));
					if (pendingRequestPtr->responseRequired()) {
						_senderRequester.sendResponse(OkResponseMessage());
					}
					break;
				} else if (pendingRequestPtr->request().instanceOf<ConnectRequestMessage>()) {
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Connect request has been received by the sender thread"));
					if (pendingRequestPtr->responseRequired()) {
						_senderRequester.sendResponse(OkResponseMessage());
					}
					connected = true;
					onSenderConnected(_socket);
				} else if (pendingRequestPtr->request().instanceOf<DisconnectRequestMessage>()) {
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Disconnect request has been received by the sender thread"));
					if (pendingRequestPtr->responseRequired()) {
						_senderRequester.sendResponse(OkResponseMessage());
					}
					if (connected) {
						connected = false;
						onSenderDisconnected(true);
					}
				} else if (!onSenderRequest(pendingRequestPtr->request())) {
					std::ostringstream msg;
					msg << "Unknown message has been received by the receiver thread: \"" << pendingRequestPtr->request().name() << '"';
					warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
				}
			}
			if (!connected) {
				continue;
			}
			if (sendingMessage) {
				try {
					if (sendMessage(*currentMessageAutoPtr.get(), _socket, _clockTimeout)) {
						sendingMessage = false;
					}
				} catch (Exception& e) {
					if (e.error().instanceOf<TcpSocket::ConnectionAbortedError>()) {
						connected = false;
					} else {
						throw;
					}
				}
				if (!connected) {
					isl::errorLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection has been aborted in the sender thread"));
					onSenderDisconnected(true);
				}
			} else if (_consumeBuffer.empty()) {
				// Fetching all messages from the input to the consume buffer
				size_t consumedMessagesAmount = inputQueue().popAll(_consumeBuffer, _clockTimeout);
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
		if (connected) {
			onSenderDisconnected(false);
		}
	}

	TcpAddrInfo _remoteAddr;
	std::auto_ptr<MessageQueueType> _inputQueueAutoPtr;
	MessageQueueType * _providedInputQueuePtr;
	std::auto_ptr<MessageBusType> _outputBusAutoPtr;
	MessageBusType * _providedOutputBusPtr;
	const Timeout _clockTimeout;
	InterThreadRequesterType _receiverRequester;
	InterThreadRequesterType _senderRequester;
	TransmitterThread _receiverThread;
	TransmitterThread _senderThread;
	TcpSocket _socket;
	MessageBufferType _consumeBuffer;
	ProvidersContainer _providers;
	ConsumersContainer _consumers;
};

//template <typename Msg, typename Cloner = CopyMessageCloner<Msg> > class AbstractMessageBrokerConnection : public Subsystem
//{
//public:
//	typedef Msg MessageType;							//!< Message type
//	typedef MessageProvider<MessageType> MessageProviderType;			//!< Message provider type
//	typedef AbstractMessageConsumer<MessageType> AbstractMessageConsumerType;	//!< Abstract message consumer type
//	typedef MessageQueue<MessageType, Cloner> MessageQueueType;			//!< Message queue type
//	typedef MessageBuffer<MessageType, Cloner> MessageBufferType;			//!< Message buffer type
//	typedef MessageBus<MessageType> MessageBusType;					//!< Message bus type
//
//	//! Input message queue factory base class
//	class InputQueueFactory
//	{
//	public:
//		virtual ~InputQueueFactory()
//		{}
//		//! Input message queue creation factory method
//		virtual MessageQueueType * create() const
//		{
//			return new MessageQueueType();
//		}
//	};
//
//	//! Output message bus factory base class
//	class OutputBusFactory
//	{
//	public:
//		virtual ~OutputBusFactory()
//		{}
//		//! Output message bus creation factory method
//		virtual MessageBusType * create() const
//		{
//			return new MessageBusType();
//		}
//	};
//
//	//! Constructor
//	/*!
//	  \param owner Pointer to the owner subsystem
//	  \param remoteAddr Message broker remote address info
//	  \param clockTimeout Subsystem's clock timeout
//	  \param inputQueueFactory Input message queue factory object reference
//	  \param outputBusFactory Output message bus factory object reference
//	*/
//	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& remoteAddr,
//			const Timeout& clockTimeout = Timeout::defaultTimeout(), const InputQueueFactory& inputQueueFactory = InputQueueFactory(),
//			const OutputBusFactory& outputBusFactory = OutputBusFactory()) :
//		Subsystem(owner),
//		_remoteAddr(remoteAddr),
//		_inputQueueAutoPtr(inputQueueFactory.create()),
//		_providedInputQueuePtr(),
//		_outputBusAutoPtr(outputBusFactory.create()),
//		_providedOutputBusPtr(),
//		_clockTimeout(clockTimeout),
//		_cond(),
//		_shouldTerminate(false),
//		_connected(false),
//		_receiverThread(),
//		_senderThread(),
//		_socket(),
//		_consumeBuffer(),
//		_providers(),
//		_consumers()
//	{}
//	//! Constructor with user provided input message queue
//	/*!
//	  \param owner Pointer to the owner subsystem
//	  \param remoteAddr Message broker remote address info
//	  \param inputQueue Reference to the input message queue to fetch messages from
//	  \param clockTimeout Subsystem's clock timeout
//	  \param outputBusFactory Output message bus factory object reference
//	*/
//	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& remoteAddr, MessageQueueType& inputQueue,
//			const Timeout& clockTimeout = Timeout::defaultTimeout(), const OutputBusFactory& outputBusFactory = OutputBusFactory()) :
//		Subsystem(owner),
//		_remoteAddr(remoteAddr),
//		_inputQueueAutoPtr(),
//		_providedInputQueuePtr(&inputQueue),
//		_outputBusAutoPtr(outputBusFactory.create()),
//		_providedOutputBusPtr(),
//		_clockTimeout(clockTimeout),
//		_cond(),
//		_shouldTerminate(false),
//		_connected(false),
//		_receiverThread(),
//		_senderThread(),
//		_socket(),
//		_consumeBuffer(),
//		_providers(),
//		_consumers()
//	{}
//	//! Constructor with user provided output message bus
//	/*!
//	  \param owner Pointer to the owner subsystem
//	  \param remoteAddr Message broker remote address info
//	  \param outputBus Reference to the output message bus to provide all received messages to
//	  \param clockTimeout Subsystem's clock timeout
//	  \param inputQueueFactory Input message queue factory object reference
//	*/
//	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& remoteAddr, MessageBusType& outputBus,
//			const Timeout& clockTimeout = Timeout::defaultTimeout(), const InputQueueFactory& inputQueueFactory = InputQueueFactory()) :
//		Subsystem(owner),
//		_remoteAddr(remoteAddr),
//		_inputQueueAutoPtr(inputQueueFactory.create()),
//		_providedInputQueuePtr(),
//		_outputBusAutoPtr(),
//		_providedOutputBusPtr(&outputBus),
//		_clockTimeout(clockTimeout),
//		_cond(),
//		_shouldTerminate(false),
//		_connected(false),
//		_receiverThread(),
//		_senderThread(),
//		_socket(),
//		_consumeBuffer(),
//		_providers(),
//		_consumers()
//	{}
//	//! Constructor with user provided input message queue and output message bus
//	/*!
//	  \param owner Pointer to the owner subsystem
//	  \param remoteAddr Message broker remote address info
//	  \param inputQueue Reference to the input message queue to fetch messages from
//	  \param outputBus Reference to the output message bus to provide all received messages to
//	  \param clockTimeout Subsystem's clock timeout
//	*/
//	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& remoteAddr, MessageQueueType& inputQueue,
//			MessageBusType& outputBus, const Timeout& clockTimeout = Timeout::defaultTimeout()) :
//		Subsystem(owner),
//		_remoteAddr(remoteAddr),
//		_inputQueueAutoPtr(),
//		_providedInputQueuePtr(&inputQueue),
//		_outputBusAutoPtr(),
//		_providedOutputBusPtr(&outputBus),
//		_clockTimeout(clockTimeout),
//		_cond(),
//		_shouldTerminate(false),
//		_connected(false),
//		_receiverThread(),
//		_senderThread(),
//		_socket(),
//		_consumeBuffer(),
//		_providers(),
//		_consumers()
//	{}
//	//! Returns a reference to the input message queue
//	inline MessageQueueType& inputQueue()
//	{
//		if (_providedInputQueuePtr) {
//			return *_providedInputQueuePtr;
//		} else if (_inputQueueAutoPtr.get()) {
//			return *_inputQueueAutoPtr.get();
//		} else {
//			throw Exception(Error(SOURCE_LOCATION_ARGS, "Input message queue have not been initialized"));
//		}
//	}
//	//! Returns a reference to the output message bus
//	inline MessageBusType& outputBus()
//	{
//		if (_providedOutputBusPtr) {
//			return *_providedOutputBusPtr;
//		} else if (_outputBusAutoPtr.get()) {
//			return *_outputBusAutoPtr.get();
//		} else {
//			throw Exception(Error(SOURCE_LOCATION_ARGS, "Output message bus have not been initialized"));
//		}
//	}
//	//! Returns message broker remote address
//	TcpAddrInfo remoteAddr() const
//	{
//		return _remoteAddr;
//	}
//	//! Sets message broker remote address
//	/*!
//	  \param newValue New message broker address
//
//	  \note Thread-unsafe: call it when subsystem is idling only
//	*/
//	void setRemoteAddr(const TcpAddrInfo& newValue)
//	{
//		_remoteAddr = newValue;
//	}
//	//! Adds message provider to subscribe input queue to while running
//	/*!
//	  \param provider Reference to provider to add
//
//	  \note Thread-unsafe: call it when subsystem is idling only
//	*/
//	void addProvider(MessageProviderType& provider)
//	{
//		_providers.push_back(&provider);
//	}
//	//! Removes message provider
//	/*!
//	  \param provider Reference to provider to remove
//
//	  \note Thread-unsafe: call it when subsystem is idling only
//	*/
//	void removeProvider(MessageProviderType& provider)
//	{
//		typename ProvidersContainer::iterator pos = std::find(_providers.begin(), _providers.end(), &provider);
//		if (pos == _providers.end()) {
//			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message provider not found in connection"));
//			return;
//		}
//		_providers.erase(pos);
//	}
//	//! Removes all message providers
//	/*!
//	  \note Thread-unsafe: call it when subsystem is idling only
//	*/
//	void resetProviders()
//	{
//		_providers.clear();
//	}
//	//! Adds message consumer for providing incoming messages to while running
//	/*!
//	  \param consumer Reference to consumer to add
//
//	  \note Thread-unsafe: call it when subsystem is idling only
//	*/
//	void addConsumer(AbstractMessageConsumerType& consumer)
//	{
//		_consumers.push_back(&consumer);
//	}
//	//! Removes message consumer
//	/*!
//	  \param consumer Reference to consumer to remove
//
//	  \note Thread-unsafe: call it when subsystem is idling only
//	*/
//	void removeConsumer(AbstractMessageConsumerType& consumer)
//	{
//		typename ConsumersContainer::iterator pos = std::find(_consumers.begin(), _consumers.end(), &consumer);
//		if (pos == _consumers.end()) {
//			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message consumer not found in connection"));
//			return;
//		}
//		_consumers.erase(pos);
//	}
//	//! Removes all message consumers
//	/*!
//	  \note Thread-unsafe: call it when subsystem is idling only
//	*/
//	void resetConsumers()
//	{
//		_consumers.clear();
//	}
//	//! Returns clock timeout
//	Timeout clockTimeout() const
//	{
//		return _clockTimeout;
//	}
//	//! Sets clock timeout
//	/*!
//	  \param newValue New clock timeout
//
//	  \note Thread-unsafe: call it when subsystem is idling only
//	*/
//	void setClockTimeout(const Timeout& newValue)
//	{
//		_clockTimeout = newValue;
//	}
//	//! Sends a message to message broker
//	inline bool sendMessage(const MessageType& msg)
//	{
//		return inputQueue().push(msg);
//	}
//	//! Sends a request message to message broker and waits for response(-s)
//	/*!
//	  \param request Constant reference to request message to send
//	  \param responseQueue Reference to response-filtering message queue to save a response(-s) to
//	  \param timeout Timeout to wait for response
//	  \return True if the message has been accepted by input message queue
//	*/
//	inline bool sendRequest(const MessageType& request, MessageQueueType& responseQueue, const Timeout& timeout = Timeout::defaultTimeout())
//	{
//		responseQueue.clear();
//		typename MessageProviderType::Subscriber subscriber(outputBus(), responseQueue);
//		if (!inputQueue().push(request)) {
//			return false;
//		}
//		return responseQueue.await(timeout);
//	}
//protected:
//	//! Sets a new value to the should terminate flag
//	/*!
//	  \param newValue New should terminate flag's value
//	*/
//	void setShouldTerminate(bool newValue)
//	{
//		MutexLocker locker(_cond.mutex());
//		_shouldTerminate = true;
//		_cond.wakeAll();
//	}
//
//	//! On connected event handler which is to be called in message receiver thread
//	/*!
//	  \param socket Reference to the connected socket
//	*/
//	virtual void onReceiverConnected(TcpSocket& socket)
//	{}
//	//! On disconnected event handler which is to be called in message receiver thread
//	/*!
//	  \param isConnectionAborted TRUE if the connection has been aborted
//	*/
//	virtual void onReceiverDisconnected(bool isConnectionAborted)
//	{}
//	//! On connect exception event handler which is to be called in message receiver thread
//	/*!
//	  \param e Constant reference to the exception object
//	  \param connectionAttempts Current unsuccessful connection attempts amount
//	*/
//	void onConnectException(const Exception& e, size_t connectionAttempts)
//	{}
//	//! On receive message from transport event handler
//	/*!
//	  Default implementation does nothing and returns true
//
//	  \param msg Constant reference to the received message
//	  \return TRUE If to proceed with the message or false to discard it
//	*/
//	virtual bool onReceiveMessage(const MessageType& msg)
//	{
//		return true;
//	}
//	//! On provide incoming message to the consumer event handler
//	/*!
//	  \param msg Constant reference to the provided message
//	  \param consumer Reference to the message consumer where the message has been provided to
//	*/
//	virtual void onProvideMessage(const MessageType& msg, AbstractMessageConsumerType& consumer)
//	{}
//	//! On connected event handler which is to be called in message sender thread
//	/*!
//	  \param socket Reference to the connected socket
//	*/
//	virtual void onSenderConnected(TcpSocket& socket)
//	{}
//	//! On disconnected event handler which is to be called in message sender thread
//	/*!
//	  \param isConnectionAborted TRUE if the connection has been aborted
//	*/
//	virtual void onSenderDisconnected(bool isConnectionAborted)
//	{}
//	//! On consume message from any provider event handler
//	/*!
//	  Default implementation does nothing and returns true
//
//	  \param msg Constant reference to the consumed message
//	  \return True if to proceed with the message or false to discard it
//	*/
//	virtual bool onConsumeMessage(const MessageType& msg)
//	{
//		return true;
//	}
//	//! On send message to transport event handler
//	/*!
//	  \param msg Constant reference to the message has been sent
//	*/
//	virtual void onSendMessage(const MessageType& msg)
//	{}
//	//! Starting subsystem method redefinition
//	virtual void startImpl()
//	{
//		// Calling ancestor's method
//		Subsystem::startImpl();
//		_shouldTerminate = false;
//		_connected = false;
//		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Starting transmitter threads"));
//		_receiverThread.start(*this, &AbstractMessageBrokerConnection<Msg, Cloner>::receive);
//		_senderThread.start(*this, &AbstractMessageBrokerConnection<Msg, Cloner>::send);
//		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Transmitter threads have been started"));
//	}
//	//! Stopting subsystem method redefinition
//	virtual void stopImpl()
//	{
//		// Stopping transmitter threads
//		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Stopping transmitter threads"));
//		setShouldTerminate(true);
//		_senderThread.join();
//		_receiverThread.join();
//		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Transmitter threads have been stopped"));
//		// Calling ancestor's method
//		Subsystem::stopImpl();
//	}
//
//	//! Receiving message from transport abstract method
//	/*!
//	  \param socket Socket to read data from
//	  \param timeout Data read timeout
//	  \return Pointer to the received message or to 0 if no message has been received
//	*/
//	virtual MessageType * receiveMessage(TcpSocket& socket, const Timeout& timeout) = 0;
//	//! Sending message to transport abstract method
//	/*!
//	  \param msg Constant reference to message to send
//	  \param socket Socket to send data to
//	  \param timeout Data send timeout
//	  \return True if the message has been sent
//	*/
//	virtual bool sendMessage(const MessageType& msg, TcpSocket& socket, const Timeout& timeout) = 0;
//private:
//	AbstractMessageBrokerConnection();
//	AbstractMessageBrokerConnection(const AbstractMessageBrokerConnection&);						// No copy
//
//	AbstractMessageBrokerConnection& operator=(const AbstractMessageBrokerConnection&);					// No copy
//
//	typedef MemFunThread<AbstractMessageBrokerConnection<Msg, Cloner> > TransmitterThread;
//	typedef std::list<MessageProviderType *> ProvidersContainer;
//	typedef std::list<AbstractMessageConsumerType *> ConsumersContainer;
//
//	void receive()
//	{
//		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Receiver thread has been started"));
//		_socket.open();
//		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Socket has been opened"));
//		bool connected = false;
//		size_t connectionAttempts = 0;
//		while (true) {
//			bool shouldTerminate;
//			{
//				MutexLocker locker(_cond.mutex());
//				if (_shouldTerminate) {
//					shouldTerminate = true;
//				} else if (!connected && connectionAttempts > 0) {
//					_cond.wait(_clockTimeout);		// Sleep for a while after unsuccessful establishing connection attempt
//					shouldTerminate = _shouldTerminate;
//				}
//			}
//			if (shouldTerminate) {
//				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection termination has been detected -> exiting from the receiver thread"));
//				break;
//			}
//			if (connected) {
//				// Receiving message if connected
//				std::auto_ptr<MessageType> msgAutoPtr;
//				try {
//					msgAutoPtr.reset(receiveMessage(_socket, _clockTimeout));
//				} catch (Exception& e) {
//					if (e.error().instanceOf<TcpSocket::ConnectionAbortedError>()) {
//						// Terminating the task if the connection has been aborted
//						isl::errorLog().log(isl::ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Message broker connection has been aborted"));
//						_socket.close();
//						onReceiverDisconnected(true);
//						connected = false;
//						connectionAttempts = 0;
//						_socket.open();
//						MutexLocker locker(_cond.mutex());
//						_connected = false;
//						_cond.wakeOne();
//					} else {
//						throw;
//					}
//				}
//				if (msgAutoPtr.get()) {
//					isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Message has been received by the receiver thread execution"));
//					// Calling on receive message event callback
//					if (!onReceiveMessage(*msgAutoPtr.get())) {
//						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by the on receive event handler"));
//						continue;
//					}
//					// Providing message to the internal output message bus
//					if (outputBus().push(*msgAutoPtr.get())) {
//						onProvideMessage(*msgAutoPtr.get(), outputBus());
//					}
//					// Providing message to all consumers
//					for (typename ConsumersContainer::iterator i = _consumers.begin(); i != _consumers.end(); ++i) {
//						if ((*i)->push(*msgAutoPtr.get())) {
//							onProvideMessage(*msgAutoPtr.get(), **i);
//						}
//					}
//				}
//			} else {
//				// Establishing connection if not connected
//				try {
//					_socket.connect(_remoteAddr);
//					connected = true;
//					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection has been established"));
//					onReceiverConnected(_socket);
//					MutexLocker locker(_cond.mutex());
//					_connected = true;
//					_cond.wakeOne();
//				} catch (Exception& e) {
//					++connectionAttempts;
//					if (connectionAttempts % 10 == 0) {
//						errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Establishing message broker connection error"));
//					}
//					onConnectException(e, connectionAttempts);
//				}
//			}
//		}
//		if (connected) {
//			_socket.close();
//			isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection has been closed"));
//			onReceiverDisconnected(false);
//		}
//	}
//
//	void send()
//	{
//		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Sender thread has been started"));
//		std::auto_ptr<MessageType> currentMessageAutoPtr;
//		bool sendingMessage = false;
//		bool connected = false;
//		// Susbcribing input message queue to the providers
//		typename MessageProviderType::SubscriberListReleaser subscriberListReleaser;
//		for (typename ProvidersContainer::iterator i = _providers.begin(); i != _providers.end(); ++i) {
//			std::auto_ptr<typename MessageProviderType::Subscriber> subscriberAutoPtr(new typename MessageProviderType::Subscriber(**i, inputQueue()));
//			subscriberListReleaser.addSubscriber(subscriberAutoPtr.get());
//			subscriberAutoPtr.release();
//			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Input queue has been subscribed to the message provider"));
//		}
//		while (true) {
//			bool shouldTerminate;
//			bool nowConnected;
//			{
//				MutexLocker locker(_cond.mutex());
//				if (_shouldTerminate) {
//					shouldTerminate = true;
//					nowConnected = _connected;
//				} else if (_connected) {
//					_cond.wait(_clockTimeout);		// Sleep for a while if not connected
//					shouldTerminate = _shouldTerminate;
//					nowConnected = _connected;
//				}
//			}
//			if (shouldTerminate) {
//				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection termination has been detected -> exiting from the sender thread"));
//				break;
//			}
//			if (connected && !nowConnected) {
//				onSenderDisconnected(true);
//			} else if (!connected && nowConnected) {
//				onSenderConnected(_socket);
//			}
//			connected = nowConnected;
//			if (!connected) {
//				continue;
//			}
//			if (sendingMessage) {
//				try {
//					if (sendMessage(*currentMessageAutoPtr.get(), _socket, _clockTimeout)) {
//						sendingMessage = false;
//					}
//				} catch (Exception& e) {
//					if (e.error().instanceOf<TcpSocket::ConnectionAbortedError>()) {
//						connected = false;
//						onSenderDisconnected(true);
//						MutexLocker locker(_cond.mutex());
//						_connected = false;
//					} else {
//						throw;
//					}
//				}
//			} else if (_consumeBuffer.empty()) {
//				// Fetching all messages from the input to the consume buffer
//				size_t consumedMessagesAmount = inputQueue().popAll(_consumeBuffer, _clockTimeout);
//				if (consumedMessagesAmount > 0) {
//					std::ostringstream oss;
//					oss << consumedMessagesAmount << " message(s) has been fetched from the input queue to the consume buffer";
//					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
//				}
//			} else {
//				// Fetching next message from the consume buffer
//				currentMessageAutoPtr = _consumeBuffer.pop();
//				if (onConsumeMessage(*currentMessageAutoPtr.get())) {
//					sendingMessage = true;
//				} else {
//					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by the on consume event handler"));
//				}
//			}
//		}
//		if (connected) {
//			onSenderDisconnected(false);
//		}
//	}
//
//	TcpAddrInfo _remoteAddr;
//	std::auto_ptr<MessageQueueType> _inputQueueAutoPtr;
//	MessageQueueType * _providedInputQueuePtr;
//	std::auto_ptr<MessageBusType> _outputBusAutoPtr;
//	MessageBusType * _providedOutputBusPtr;
//	const Timeout _clockTimeout;
//	mutable WaitCondition _cond;
//	bool _shouldTerminate;
//	bool _connected;
//	TransmitterThread _receiverThread;
//	TransmitterThread _senderThread;
//	TcpSocket _socket;
//	MessageBufferType _consumeBuffer;
//	ProvidersContainer _providers;
//	ConsumersContainer _consumers;
//};

} // namespace isl

#endif

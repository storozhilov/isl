#ifndef ISL__ABSTRACT_MESSAGE_BROKER_CONNECTION__HXX
#define ISL__ABSTRACT_MESSAGE_BROKER_CONNECTION__HXX

#include <isl/ThreadRequesterSubsystem.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/TcpAddrInfo.hxx>
#include <isl/IOError.hxx>
#include <isl/MessageQueue.hxx>
#include <isl/MessageBuffer.hxx>
#include <isl/MessageProvider.hxx>
#include <isl/MessageBus.hxx>
#include <isl/Error.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>
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
template <typename Msg, typename Cloner = CopyMessageCloner<Msg> > class AbstractMessageBrokerConnection : public ThreadRequesterSubsystem
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
		ThreadRequesterSubsystem(owner, clockTimeout),
		_remoteAddr(remoteAddr),
		_inputQueueAutoPtr(inputQueueFactory.create()),
		_providedInputQueuePtr(),
		_outputBusAutoPtr(outputBusFactory.create()),
		_providedOutputBusPtr(),
		_receiverThread(*this),
		_senderThread(*this),
		_socket(),
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
		ThreadRequesterSubsystem(owner, clockTimeout),
		_remoteAddr(remoteAddr),
		_inputQueueAutoPtr(),
		_providedInputQueuePtr(&inputQueue),
		_outputBusAutoPtr(outputBusFactory.create()),
		_providedOutputBusPtr(),
		_receiverThread(*this),
		_senderThread(*this),
		_socket(),
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
		ThreadRequesterSubsystem(owner, clockTimeout),
		_remoteAddr(remoteAddr),
		_inputQueueAutoPtr(inputQueueFactory.create()),
		_providedInputQueuePtr(),
		_outputBusAutoPtr(),
		_providedOutputBusPtr(&outputBus),
		_receiverThread(*this),
		_senderThread(*this),
		_socket(),
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
		ThreadRequesterSubsystem(owner, clockTimeout),
		_remoteAddr(remoteAddr),
		_inputQueueAutoPtr(),
		_providedInputQueuePtr(&inputQueue),
		_outputBusAutoPtr(),
		_providedOutputBusPtr(&outputBus),
		_receiverThread(*this),
		_senderThread(*this),
		_socket(),
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
	const TcpAddrInfo& remoteAddr() const
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
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Message provider not found in connection"));
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
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Message consumer not found in connection"));
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
	//! Sends a message to message broker
	inline bool sendMessage(const MessageType& msg)
	{
		return inputQueue().push(msg);
	}
	//! Sends a request message to message broker and waits for response(-s)
	/*!
	  \param request Constant reference to request message to send
	  \param responseQueue Reference to response-filtering message queue to save a response(-s) to
	  \param limit Time limit to wait for response
	  \return True if the message has been accepted by input message queue
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
	//! On receive message thread overload event handler
	/*!
	  \param Ticks expired (always > 2)
	*/
	virtual void onReceiverOverload(size_t ticksExpired)
	{}
	//! On connected event handler which is to be called in message receiver thread
	/*!
	  \param socket Reference to the connection socket
	*/
	virtual void onReceiverConnected(TcpSocket& socket)
	{}
	//! On disconnected event handler which is to be called in message receiver thread
	/*!
	  \param isConnectionAborted TRUE if the connection has been aborted
	*/
	virtual void onReceiverDisconnected(bool isConnectionAborted)
	{}
	//! On failed establishing connection attempt event handler
	/*!
	  \param failedAttempts Current unsuccessful connection attempts amount
	  \param e Constant reference to the exception object
	*/
	virtual void onConnectFailed(size_t failedAttempts, const Exception& e)
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
	//! On provide incoming message to the consumer in the receiver thread event handler
	/*!
	  \param msg Constant reference to the provided message
	  \param consumer Reference to the message consumer where the message has been provided to
	*/
	virtual void onProvideMessage(const MessageType& msg, AbstractMessageConsumerType& consumer)
	{}
	//! On send message thread overload event handler
	/*!
	  \param Ticks expired (always > 2)
	*/
	virtual void onSenderOverload(size_t ticksExpired)
	{}
	//! On connected event handler which is to be called in message sender thread
	/*!
	  \param socket Reference to the connection socket
	*/
	virtual void onSenderConnected(TcpSocket& socket)
	{}
	//! On disconnected event handler which is to be called in message sender thread
	/*!
	  \param isConnectionAborted TRUE if the connection has been aborted
	*/
	virtual void onSenderDisconnected(bool isConnectionAborted)
	{}
	//! On consume message from any provider in the receiver thread event handler
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

	//! Receiving message from transport abstract method
	/*!
	  \param socket Socket to read data from
	  \param limit Data read time limit
	  \return Pointer to the received message or to 0 if no message has been received
	*/
	virtual MessageType * receiveMessage(TcpSocket& socket, const Timestamp& limit) = 0;
	//! Sending message to transport abstract method
	/*!
	  \param msg Constant reference to message to send
	  \param socket Socket to send data to
	  \param limit Data send time limit
	  \return True if the message has been sent
	*/
	virtual bool sendMessage(const MessageType& msg, TcpSocket& socket, const Timestamp& limit) = 0;
private:
	AbstractMessageBrokerConnection();
	AbstractMessageBrokerConnection(const AbstractMessageBrokerConnection&);						// No copy

	AbstractMessageBrokerConnection& operator=(const AbstractMessageBrokerConnection&);					// No copy

	typedef std::list<MessageProviderType *> ProvidersContainer;
	typedef std::list<AbstractMessageConsumerType *> ConsumersContainer;

	class ConnectRequest : public AbstractThreadMessage
	{
	public:
		virtual const char * name() const
		{
			static const char * n = "Connect request";
			return n;
		}
		virtual AbstractThreadMessage * clone() const
		{
			return new ConnectRequest(*this);
		}
	};

	class DisconnectRequest : public AbstractThreadMessage
	{
	public:
		virtual const char * name() const
		{
			static const char * n = "Disconnect request";
			return n;
		}
		virtual AbstractThreadMessage * clone() const
		{
			return new DisconnectRequest(*this);
		}
	};

	//! Message receiver thread class
	class ReceiverThread : public Thread
	{
	public:
		ReceiverThread(AbstractMessageBrokerConnection& connection) :
			Thread(connection),
			_connection(connection),
			_connected(false),
			_connectionAttempts(0)
		{}
	private:
		//! On start event handler
		/*!
		  \return TRUE if to continue thread execution
		*/
		virtual bool onStart()
		{
			_connection._socket.open();
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Socket has been opened"));
			_connected = false;
			_connectionAttempts = 0;
			return true;
		}
		//! Doing the work virtual method
		/*!
		  \param limit Limit timestamp for doing the work
		  \return TRUE if to continue thread execution
		*/
		virtual bool doLoad(const Timestamp& limit)
		{
			while (Timestamp::now() < limit) {
				if (_connected) {
					// Receiving message if connected
					std::auto_ptr<MessageType> msgAutoPtr;
					try {
						msgAutoPtr.reset(_connection.receiveMessage(_connection._socket, limit));
					} catch (Exception& e) {
						if (e.error().instanceOf<TcpSocket::ConnectionAbortedError>()) {
							_connected = false;
						} else {
							throw;
						}
					}
					if (!_connected) {
						Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection has been aborted in the receiver thread"));
						_connectionAttempts = 0;
						// Sending disconnect request to the sender thread
						size_t requestId = _connection._senderThread.requester().sendRequest(DisconnectRequest());
						if (requestId > 0) {
							Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Disconnect request has been sent to the sender thread"));
						} else {
							Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Could not send disconnect request to the sender thread"));
						}
						// Calling 'on receiver disconnected' event handler
						_connection.onReceiverDisconnected(true);
						// Awaiting for the response from the sender thread
						if (requestId > 0) {
							// TODO Use proper timeout to wait for the response
							std::auto_ptr<AbstractThreadMessage> responseAutoPtr =
								_connection._senderThread.requester().awaitResponse(requestId, Timestamp::limit(_connection.awaitResponseTimeout()));
							if (!responseAutoPtr.get()) {
								Log::error().log(LogMessage(SOURCE_LOCATION_ARGS,
											"No response to disconnect request have been received from the sender thread"));
							} else if (responseAutoPtr->instanceOf<OkResponse>()) {
								Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
											"OK response to the disconnect request has been received from the sender thread"));
							} else {
								Log::error().log(LogMessage(SOURCE_LOCATION_ARGS,
										"Invalid response to disconnect request has been received from the sender thread: \"") <<
										responseAutoPtr->name() << "\"");
							}
						}
						// Reopening socket
						_connection._socket.close();
						_connection._socket.open();
					} else if (msgAutoPtr.get()) {
						isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Message has been received by the receiver thread execution"));
						// Calling on receive message event callback
						if (!_connection.onReceiveMessage(*msgAutoPtr.get())) {
							Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by the on receive event handler"));
							continue;
						}
						// Providing message to the internal output message bus
						if (_connection.outputBus().push(*msgAutoPtr.get())) {
							_connection.onProvideMessage(*msgAutoPtr.get(), _connection.outputBus());
						}
						// Providing message to all consumers
						for (typename ConsumersContainer::iterator i = _connection._consumers.begin(); i != _connection._consumers.end(); ++i) {
							if ((*i)->push(*msgAutoPtr.get())) {
								_connection.onProvideMessage(*msgAutoPtr.get(), **i);
							}
						}
					}
				} else {
					// Establishing connection if not connected
					try {
						_connection._socket.connect(_connection._remoteAddr);
						Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection has been established"));
						_connected = true;
						// Sending connect request to the sender thread
						size_t requestId = _connection._senderThread.requester().sendRequest(ConnectRequest());
						if (requestId > 0) {
							Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Connect request has been sent to the sender thread"));
						} else {
							Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Could not send connect request to the sender thread"));
						}
						// Calling 'on receiver connected' event handler
						_connection.onReceiverConnected(_connection._socket);
						// Awaiting for the response from the sender thread
						if (requestId > 0) {
							// TODO Use proper timeout to wait for the response
							std::auto_ptr<AbstractThreadMessage> responseAutoPtr =
								_connection._senderThread.requester().awaitResponse(requestId, Timestamp::limit(_connection.awaitResponseTimeout()));
							if (!responseAutoPtr.get()) {
								Log::error().log(LogMessage(SOURCE_LOCATION_ARGS,
											"No response to connect request have been received from the sender thread"));
							} else if (responseAutoPtr->instanceOf<OkResponse>()) {
								Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
											"OK response to the connect request has been received from the sender thread"));
							} else {
								Log::error().log(LogMessage(SOURCE_LOCATION_ARGS,
										"Invalid response to connect request has been received from the sender thread: \"") <<
										responseAutoPtr->name() << "\"");
							}
						}
					} catch (Exception& e) {
						++_connectionAttempts;
						_connection.onConnectFailed(_connectionAttempts, e);
						break;
					}
				}
			}
			return true;
		}
		//! On overload event handler
		/*!
		  \param Ticks expired (always > 2)
		  \return TRUE if to continue thread execution
		*/
		virtual bool onOverload(size_t ticksExpired)
		{
			//return _connection.onReceiverOverload(ticksExpired);	// TODO ???
			_connection.onReceiverOverload(ticksExpired);
			return true;
		}
		//! On stop event handler
		virtual void onStop()
		{
			if (_connected) {
				_connection._socket.close();
				isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection has been closed"));
				_connection.onReceiverDisconnected(false);
			}
		}

		AbstractMessageBrokerConnection& _connection;
		bool _connected;
		size_t _connectionAttempts;
	};

	//! Message sender thread class
	class SenderThread : public Thread
	{
	public:
		SenderThread(AbstractMessageBrokerConnection& connection) :
			Thread(connection),
			_connection(connection),
			_currentMessageAutoPtr(),
			_sendingMessage(false),
			_connected(false),
			_consumeBuffer(),
			_subscriberListReleaserAutoPtr()
		{}
	private:
		//! On thread request event handler
		/*!
		  \param pendingRequest Constant reference to pending resuest to process
		*/
		virtual void onThreadRequest(const ThreadRequesterType::PendingRequest& pendingRequest)
		{
			if (pendingRequest.request().instanceOf<ConnectRequest>()) {
				Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Connect request has been received by the sender thread"));
				if (pendingRequest.responseRequired()) {
					requester().sendResponse(OkResponse());
				}
				_connected = true;
				_connection.onSenderConnected(_connection._socket);
			} else if (pendingRequest.request().instanceOf<DisconnectRequest>()) {
				Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Disconnect request has been received by the sender thread"));
				if (pendingRequest.responseRequired()) {
					requester().sendResponse(OkResponse());
				}
				if (_connected) {
					_connected = false;
					_connection.onSenderDisconnected(true);
				}
			}
		}
		//! On start event handler
		/*!
		  \return TRUE if to continue thread execution
		*/
		virtual bool onStart()
		{
			_currentMessageAutoPtr.reset();
			_sendingMessage = false;
			_connected = false;
			_consumeBuffer.clear();
			// Susbcribing input message queue to the providers
			_subscriberListReleaserAutoPtr.reset(new typename MessageProviderType::SubscriberListReleaser());
			for (typename ProvidersContainer::iterator i = _connection._providers.begin(); i != _connection._providers.end(); ++i) {
				std::auto_ptr<typename MessageProviderType::Subscriber> subscriberAutoPtr(new typename MessageProviderType::Subscriber(**i, _connection.inputQueue()));
				_subscriberListReleaserAutoPtr->addSubscriber(subscriberAutoPtr.get());
				subscriberAutoPtr.release();
				Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Input queue has been subscribed to the message provider"));
			}
			return true;
		}
		//! Doing the work virtual method
		/*!
		  \param limit Limit timestamp for doing the work
		  \return TRUE if to continue thread execution
		*/
		virtual bool doLoad(const Timestamp& limit)
		{
			if (_connected) {
				while (Timestamp::now() < limit) {
					if (_sendingMessage) {
						try {
							if (_connection.sendMessage(*_currentMessageAutoPtr.get(), _connection._socket, limit)) {
								_sendingMessage = false;
							}
						} catch (Exception& e) {
							if (e.error().instanceOf<TcpSocket::ConnectionAbortedError>()) {
								_connected = false;
							} else {
								throw;
							}
						}
						if (!_connected) {
							isl::Log::error().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection has been aborted in the sender thread"));
							_connection.onSenderDisconnected(true);
						}
					} else if (_consumeBuffer.empty()) {
						// Fetching all messages from the input to the consume buffer
						size_t consumedMessagesAmount = _connection.inputQueue().popAll(_consumeBuffer, limit);
						if (consumedMessagesAmount > 0) {
							Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS) << consumedMessagesAmount <<
									" message(s) has been fetched from the input queue to the consume buffer");
						}
					} else {
						// Fetching next message from the consume buffer
						_currentMessageAutoPtr = _consumeBuffer.pop();
						if (_connection.onConsumeMessage(*_currentMessageAutoPtr.get())) {
							_sendingMessage = true;
						} else {
							Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by the on consume event handler"));
						}
					}
				}
			}
			return true;
		}
		//! On overload event handler
		/*!
		  \param Ticks expired (always > 2)
		  \return TRUE if to continue thread execution
		*/
		virtual bool onOverload(size_t ticksExpired)
		{
			//return _connection.onSenderOverload(ticksExpired);	// TODO ???
			_connection.onSenderOverload(ticksExpired);
			return true;
		}
		//! On stop event handler
		virtual void onStop()
		{
			_currentMessageAutoPtr.reset();
			_subscriberListReleaserAutoPtr.reset();
			if (_connected) {
				_connection.onSenderDisconnected(false);
			}
		}

		AbstractMessageBrokerConnection& _connection;
		std::auto_ptr<MessageType> _currentMessageAutoPtr;
		bool _sendingMessage;
		bool _connected;
		MessageBufferType _consumeBuffer;
		std::auto_ptr<typename MessageProviderType::SubscriberListReleaser> _subscriberListReleaserAutoPtr;
	};

	TcpAddrInfo _remoteAddr;
	std::auto_ptr<MessageQueueType> _inputQueueAutoPtr;
	MessageQueueType * _providedInputQueuePtr;
	std::auto_ptr<MessageBusType> _outputBusAutoPtr;
	MessageBusType * _providedOutputBusPtr;
	ReceiverThread _receiverThread;
	SenderThread _senderThread;
	TcpSocket _socket;
	ProvidersContainer _providers;
	ConsumersContainer _consumers;
};

} // namespace isl

#endif

#ifndef ISL__ABSTRACT_MESSAGE_BROKER_LISTENING_CONNECTION__HXX
#define ISL__ABSTRACT_MESSAGE_BROKER_LISTENING_CONNECTION__HXX

#include <isl/Subsystem.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/TcpAddrInfo.hxx>
#include <isl/IOError.hxx>
#include <isl/ReadWriteLock.hxx>
#include <isl/MessageQueue.hxx>
#include <isl/MessageBuffer.hxx>
#include <isl/MessageProvider.hxx>
#include <isl/MessageBus.hxx>
#include <isl/Error.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <memory>

namespace isl
{

//! Message broker listening connection subsystem abstract templated class
/*!
  TODO: Remove event handlers from the connection class and make receiver & sender threads to be created using factory method.

  Use this class for your message broker listening connection implementation. It creates two threads:
  one is for receiving messages and another is for sending ones. So, you should define
  following two abstract virtual methods in your subclass:

  - AbstractMessageBrokerListeningConnection::receiveMessage() - receives message from the transport;
  - AbstractMessageBrokerListeningConnection::sendMessage() - sends message to the transport.

  TCP-connection control is provided by message receiver thread, which is automatically re-accepts it if aborted.
  A thrown Exception with TcpSocket::ConnectionAbortedError error from the receiveMessage()/sendMessage()
  method is used as signal for re-accepting TCP-connection socket.

  \tparam Msg Message class
  \tparam Cloner Message cloner class with static <tt>Msg * Cloner::clone(const Msg& msg)</tt> method for cloning the message
*/
template <typename Msg, typename Cloner = CopyMessageCloner<Msg> > class AbstractMessageBrokerListeningConnection : public Subsystem
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
	  \param localAddr Local address to bind to
	  \param clockTimeout Subsystem's clock timeout
	  \param inputQueueFactory Input message queue factory object reference
	  \param outputBusFactory Output message bus factory object reference
	*/
	AbstractMessageBrokerListeningConnection(Subsystem * owner, const TcpAddrInfo& localAddr,
			const Timeout& clockTimeout = Timeout::defaultTimeout(), const InputQueueFactory& inputQueueFactory = InputQueueFactory(),
			const OutputBusFactory& outputBusFactory = OutputBusFactory()) :
		Subsystem(owner, clockTimeout),
		_localAddr(localAddr),
		_inputQueueAutoPtr(inputQueueFactory.create()),
		_providedInputQueuePtr(),
		_outputBusAutoPtr(outputBusFactory.create()),
		_providedOutputBusPtr(),
		_receiverThread(*this),
		_senderThread(*this),
		_socket(),
		_transferSocketAutoPtr(),
		_providers(),
		_consumers()
	{}
	//! Constructor with user provided input message queue
	/*!
	  \param owner Pointer to the owner subsystem
	  \param localAddr Local address to bind to
	  \param inputQueue Reference to the input message queue to fetch messages from
	  \param clockTimeout Subsystem's clock timeout
	  \param outputBusFactory Output message bus factory object reference
	*/
	AbstractMessageBrokerListeningConnection(Subsystem * owner, const TcpAddrInfo& localAddr, MessageQueueType& inputQueue,
			const Timeout& clockTimeout = Timeout::defaultTimeout(), const OutputBusFactory& outputBusFactory = OutputBusFactory()) :
		Subsystem(owner, clockTimeout),
		_localAddr(localAddr),
		_inputQueueAutoPtr(),
		_providedInputQueuePtr(&inputQueue),
		_outputBusAutoPtr(outputBusFactory.create()),
		_providedOutputBusPtr(),
		_receiverThread(*this),
		_senderThread(*this),
		_socket(),
		_transferSocketAutoPtr(),
		_providers(),
		_consumers()
	{}
	//! Constructor with user provided output message bus
	/*!
	  \param owner Pointer to the owner subsystem
	  \param localAddr Local address to bind to
	  \param outputBus Reference to the output message bus to provide all received messages to
	  \param clockTimeout Subsystem's clock timeout
	  \param inputQueueFactory Input message queue factory object reference
	*/
	AbstractMessageBrokerListeningConnection(Subsystem * owner, const TcpAddrInfo& localAddr, MessageBusType& outputBus,
			const Timeout& clockTimeout = Timeout::defaultTimeout(), const InputQueueFactory& inputQueueFactory = InputQueueFactory()) :
		Subsystem(owner, clockTimeout),
		_localAddr(localAddr),
		_inputQueueAutoPtr(inputQueueFactory.create()),
		_providedInputQueuePtr(),
		_outputBusAutoPtr(),
		_providedOutputBusPtr(&outputBus),
		_receiverThread(*this),
		_senderThread(*this),
		_socket(),
		_transferSocketAutoPtr(),
		_providers(),
		_consumers()
	{}
	//! Constructor with user provided input message queue and output message bus
	/*!
	  \param owner Pointer to the owner subsystem
	  \param localAddr Local address to bind to
	  \param inputQueue Reference to the input message queue to fetch messages from
	  \param outputBus Reference to the output message bus to provide all received messages to
	  \param clockTimeout Subsystem's clock timeout
	*/
	AbstractMessageBrokerListeningConnection(Subsystem * owner, const TcpAddrInfo& localAddr, MessageQueueType& inputQueue,
			MessageBusType& outputBus, const Timeout& clockTimeout = Timeout::defaultTimeout()) :
		Subsystem(owner, clockTimeout),
		_localAddr(localAddr),
		_inputQueueAutoPtr(),
		_providedInputQueuePtr(&inputQueue),
		_outputBusAutoPtr(),
		_providedOutputBusPtr(&outputBus),
		_receiverThread(*this),
		_senderThread(*this),
		_socket(),
		_transferSocketAutoPtr(),
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
	//! Returns local address to bind to
	const TcpAddrInfo& localAddr() const
	{
		return _localAddr;
	}
	//! Sets the local address to bind to
	/*!
	  \param newValue New message broker address

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	void setLocalAddr(const TcpAddrInfo& newValue)
	{
		_localAddr = newValue;
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
	//! Enqueues a message for sending to peer
        /*!
         * \param mgs A constant reference to message to enqueue
         */
	inline bool enqueueMessage(const MessageType& msg)
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
	inline bool makeRequest(const MessageType& request, MessageQueueType& responseQueue, const Timestamp& limit)
	{
		responseQueue.clear();
		typename MessageProviderType::Subscriber subscriber(outputBus(), responseQueue);
		if (!inputQueue().push(request)) {
			return false;
		}
		return responseQueue.await(limit);
	}
	//! Starting subsystem method redefinition
protected:
	class ConnectRequest : public AbstractThreadMessage
	{
	public:
		ConnectRequest() :
			AbstractThreadMessage("Connect request")
		{}
		virtual AbstractThreadMessage * clone() const
		{
			return new ConnectRequest(*this);
		}
	};

	class DisconnectRequest : public AbstractThreadMessage
	{
	public:
		DisconnectRequest() :
			AbstractThreadMessage("Disconnect request")
		{}
		virtual AbstractThreadMessage * clone() const
		{
			return new DisconnectRequest(*this);
		}
	};

	//! Message receiver thread class
	class ReceiverThread : public OscillatorThread
	{
	public:
		ReceiverThread(AbstractMessageBrokerListeningConnection& connection) :
			OscillatorThread(connection),
			_connection(connection),
			_connected(false),
			_acceptingAttempts(0)
		{}
	private:
		//! On start event handler
		virtual void onStart()
		{
                        _connection.onReceiverStart();
			_connection._socket.open();
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Socket has been opened"));
			_connection._socket.bind(_connection._localAddr);
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Socket has been binded"));
			_connection._socket.listen(1);
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Socket has been switched to the listening state"));
			_connected = false;
			_acceptingAttempts = 0;
		}
		//! Doing the work virtual method
		/*!
		  \param prevTickTimestamp Previous tick timestamp
		  \param nextTickTimestamp Next tick timestamp
		  \param ticksExpired Amount of expired ticks - if > 1, then an overload has occured
		*/
		virtual void doLoad(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
		{
			while (Timestamp::now() < nextTickTimestamp) {
				if (_connected) {
					// Receiving message if connected
					std::auto_ptr<MessageType> msgAutoPtr;
					try {
						msgAutoPtr.reset(_connection.receiveMessage(*_connection._transferSocketAutoPtr.get(), nextTickTimestamp));
					} catch (Exception& e) {
						if (e.error().instanceOf<TcpSocket::ConnectionAbortedError>()) {
							_connected = false;
						} else {
							throw;
						}
					}
					if (!_connected) {
						isl::Log::error().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection has been aborted in the receiver thread"));
                                                _connection.onReceiverDisconnected(true);
						_acceptingAttempts = 0;
						// Sending disconnect request to the sender thread
						std::auto_ptr<ThreadRequesterType::MessageType> responseAutoPtr =
							_connection._senderThread.sendRequest(DisconnectRequest(), Timestamp::limit(_connection.awaitResponseTimeout()));
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
						// Resetting transfer socket autopointer
						_connection._transferSocketAutoPtr.reset();
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
					_connection._transferSocketAutoPtr = _connection._socket.accept(nextTickTimestamp.leftTo());
					if (_connection._transferSocketAutoPtr.get()) {
						Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Connection has been accepted from message exchange peer"));
						_connected = true;
                                                _connection.onReceiverConnected(*_connection._transferSocketAutoPtr.get());
						// Sending connect request to the sender thread
						std::auto_ptr<ThreadRequesterType::MessageType> responseAutoPtr =
							_connection._senderThread.sendRequest(ConnectRequest(), Timestamp::limit(_connection.awaitResponseTimeout()));
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
					} else {
						++_acceptingAttempts;
						_connection.onAcceptFailed(_acceptingAttempts);
						break;
					}
				}
			}
		}
		//! On overload event handler
		/*!
		  \param prevTickTimestamp Previous tick timestamp
		  \param nextTickTimestamp Next tick timestamp
		  \param Amount of expired ticks - always > 2
		*/
		virtual void onOverload(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
		{
			_connection.onReceiverOverload(prevTickTimestamp, nextTickTimestamp, ticksExpired);
		}
		//! On stop event handler
		virtual void onStop()
		{
			if (_connected) {
				_connection._transferSocketAutoPtr.reset();
				isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Connection has been closed"));
				_connection.onReceiverDisconnected(false);
			}
                        _connection.onReceiverStop();
		}

		AbstractMessageBrokerListeningConnection& _connection;
		bool _connected;
		size_t _acceptingAttempts;
	};

	//! Message sender thread class
	class SenderThread : public OscillatorThread
	{
	public:
		SenderThread(AbstractMessageBrokerListeningConnection& connection) :
			OscillatorThread(connection),
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
		  \param request Constant reference to pending request to process
                  \param responseRequired TRUE if the response is required
                  \param stopRequestsProcessing A reference to flag, which means to terminate next incoming requests processing [OUT]
		  \return Auto-pointer to the response or to 0 if no response has been provided
		*/
		virtual std::auto_ptr<ThreadRequesterType::MessageType> onRequest(const ThreadRequesterType::MessageType& request,
                                bool responseRequired, bool& stopRequestsProcessing)
		{
			if (request.instanceOf<ConnectRequest>()) {
				Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Connect request has been received by the sender thread"));
				_connected = true;
				_connection.onSenderConnected(*_connection._transferSocketAutoPtr.get());
				return std::auto_ptr<ThreadRequesterType::MessageType>(responseRequired ? new OkResponse() : 0);
			} else if (request.instanceOf<DisconnectRequest>()) {
				Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Disconnect request has been received by the sender thread"));
				if (_connected) {
					_connected = false;
					_connection.onSenderDisconnected(true);
				}
				return std::auto_ptr<ThreadRequesterType::MessageType>(responseRequired ? new OkResponse() : 0);
			}
			return OscillatorThread::onRequest(request, responseRequired, stopRequestsProcessing);
		}
		//! On start event handler
		virtual void onStart()
		{
                        _connection.onSenderStart();
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
		}
		//! Doing the work virtual method
		/*!
		  \param prevTickTimestamp Previous tick timestamp
		  \param nextTickTimestamp Next tick timestamp
		  \param ticksExpired Amount of expired ticks - if > 1, then an overload has occured
		*/
		virtual void doLoad(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
		{
			if (_connected) {
				while (Timestamp::now() < nextTickTimestamp) {
					if (_sendingMessage) {
						try {
							if (_connection.sendMessage(*_currentMessageAutoPtr.get(), *_connection._transferSocketAutoPtr.get(), nextTickTimestamp)) {
								_sendingMessage = false;
                                                                _connection.onSendMessage(*_currentMessageAutoPtr.get());
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
						size_t consumedMessagesAmount = _connection.inputQueue().popAll(_consumeBuffer, nextTickTimestamp);
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
		}
		//! On overload event handler
		/*!
		  \param prevTickTimestamp Previous tick timestamp
		  \param nextTickTimestamp Next tick timestamp
		  \param Amount of expired ticks - always > 2
		*/
		virtual void onOverload(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
		{
			_connection.onSenderOverload(prevTickTimestamp, nextTickTimestamp, ticksExpired);
		}
		//! On stop event handler
		virtual void onStop()
		{
			_currentMessageAutoPtr.reset();
			_subscriberListReleaserAutoPtr.reset();
			if (_connected) {
				_connection.onSenderDisconnected(false);
			}
                        _connection.onSenderStop();
		}

		AbstractMessageBrokerListeningConnection& _connection;
		std::auto_ptr<MessageType> _currentMessageAutoPtr;
		bool _sendingMessage;
		bool _connected;
		MessageBufferType _consumeBuffer;
		std::auto_ptr<typename MessageProviderType::SubscriberListReleaser> _subscriberListReleaserAutoPtr;
	};

        //! On receiver start event handler
        virtual void onReceiverStart()
        {}
	//! On receive message thread overload event handler
	/*!
	  \param prevTickTimestamp Previous tick timestamp
	  \param nextTickTimestamp Next tick timestamp
	  \param Amount of expired ticks - always > 2
	*/
	virtual void onReceiverOverload(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
	{}
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
	//! On failed accepting connection attempt event handler
	/*!
	  \param failedAttempts Current unsuccessful accepting connection attempts amount
	*/
	virtual void onAcceptFailed(size_t failedAttempts)
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
        //! On receiver stop event handler
        virtual void onReceiverStop()
        {}
        //! On sender start event handler
        virtual void onSenderStart()
        {}
	//! On send message thread overload event handler
	/*!
	  \param prevTickTimestamp Previous tick timestamp
	  \param nextTickTimestamp Next tick timestamp
	  \param Amount of expired ticks - always > 2
	*/
	virtual void onSenderOverload(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
	{}
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
        //! On sender stop event handler
        virtual void onSenderStop()
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
	  \param limit Data read time limit
	  \return True if the message has been sent
	*/
	virtual bool sendMessage(const MessageType& msg, TcpSocket& socket, const Timestamp& limit) = 0;
private:
	AbstractMessageBrokerListeningConnection();
	AbstractMessageBrokerListeningConnection(const AbstractMessageBrokerListeningConnection&);						// No copy

	AbstractMessageBrokerListeningConnection& operator=(const AbstractMessageBrokerListeningConnection&);					// No copy

	typedef std::list<MessageProviderType *> ProvidersContainer;
	typedef std::list<AbstractMessageConsumerType *> ConsumersContainer;

	TcpAddrInfo _localAddr;
	std::auto_ptr<MessageQueueType> _inputQueueAutoPtr;
	MessageQueueType * _providedInputQueuePtr;
	std::auto_ptr<MessageBusType> _outputBusAutoPtr;
	MessageBusType * _providedOutputBusPtr;
	ReceiverThread _receiverThread;
	SenderThread _senderThread;
	TcpSocket _socket;
	std::auto_ptr<TcpSocket> _transferSocketAutoPtr;
	ProvidersContainer _providers;
	ConsumersContainer _consumers;
};

} // namespace isl

#endif

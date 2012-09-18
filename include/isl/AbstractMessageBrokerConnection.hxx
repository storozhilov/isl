#ifndef ISL__ABSTRACT_MESSAGE_BROKER_CONNECTION__HXX
#define ISL__ABSTRACT_MESSAGE_BROKER_CONNECTION__HXX

#include <isl/common.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/TcpAddrInfo.hxx>
#include <isl/Subsystem.hxx>
#include <isl/IOError.hxx>
#include <isl/ReadWriteLock.hxx>
#include <isl/MessageQueue.hxx>
#include <isl/MessageBuffer.hxx>
#include <isl/MessageProvider.hxx>
#include <isl/MessageBus.hxx>
#include <isl/Error.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>
#include <memory>

namespace isl
{

//! Message broker connection subsystem abstract templated class
/*!
  Use this class for your message broker connection implementation. It creates 2 threads which should be
  objects of the descendants of the following abstract classes:
  
  - AbstractMessageBrokerConnection::AbstractReceiverThread - is for receiving messages from the transport
    and providing them to subscribed message consumers. Your descendant of this class should override
    AbstractMessageBrokerConnection::AbstractReceiverThread::receiveMessage() method, which actually
    receives message from the network transport;
  - AbstractMessageBrokerConnection::AbstractSenderThread - is for consuming messages from the message
    providers subscribed to and sending them to the transport. Your descendant of this class should override
    AbstractMessageBrokerConnection::AbstractSenderThread::sendMessage() method, which actually
    sends message to the network transport.

  To implement your message broker connection class you should also override following abstract factory
  methods, which are used for thread objects creation during subsystem's startup:

  - AbstractMessageBrokerConnection::createReceiverThread() - creates receiver thread object;
  - AbstractMessageBrokerConnection::createSenderThread() - creates sender thread object.

  TCP-connection is automatically reestablished during this subsystem's execution, so permanent socket is not
  available.

  \tparam Msg Message class
  \tparam Cloner Message cloner class with static <tt>Msg * Cloner::clone(const Msg& msg)</tt> method for cloning the message
*/
template <typename Msg, typename Cloner = CopyMessageCloner<Msg> > class AbstractMessageBrokerConnection : public Subsystem
{
public:
	typedef Msg MessageType;						//!< Message type
	typedef MessageProvider<Msg> MessageProviderType;			//!< Message provider type
	typedef AbstractMessageConsumer<Msg> AbstractMessageConsumerType;	//!< Abstract message consumer type
	typedef MessageQueue<Msg, Cloner> MessageQueueType;			//!< Message queue type
	typedef MessageBuffer<Msg, Cloner> MessageBufferType;			//!< Message buffer type
	typedef MessageBus<Msg> MessageBusType;					//!< Message bus type

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
	  \param addr Message broker remote address if <tt>isListening = TRUE</tt> or a local address to bind otherwise
	  \param isListening Does the receiver thread should listen for incoming connection or connect the remote address otherwise
	  \param clockTimeout Subsystem's clock timeout
	  \param inputQueueFactory Input message queue factory object reference
	  \param outputBusFactory Output message bus factory object reference
	*/
	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& addr, bool isListening = false,
			const Timeout& clockTimeout = Timeout::defaultTimeout(), const InputQueueFactory& inputQueueFactory = InputQueueFactory(),
			const OutputBusFactory& outputBusFactory = OutputBusFactory()) :
		Subsystem(owner),
		_addr(addr),
		_inputQueueAutoPtr(inputQueueFactory.create()),
		_providedInputQueuePtr(),
		_outputBusAutoPtr(outputBusFactory.create()),
		_providedOutputBusPtr(),
		_isListening(isListening),
		_clockTimeout(clockTimeout),
		_receiverThreadAutoPtr(),
		_senderThreadAutoPtr(),
		_socket(),
		_providers(),
		_consumers()
	{}
	//! Constructor with user provided input message queue
	/*!
	  \param owner Pointer to the owner subsystem
	  \param addr Message broker remote address if <tt>isListening = TRUE</tt> or a local address to bind otherwise
	  \param inputQueue Reference to the input message queue to fetch messages from
	  \param isListening Does the receiver thread should listen for incoming connection or connect the remote address otherwise
	  \param clockTimeout Subsystem's clock timeout
	  \param outputBusFactory Output message bus factory object reference
	*/
	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& addr, MessageQueueType& inputQueue, bool isListening = false,
			const Timeout& clockTimeout = Timeout::defaultTimeout(), const OutputBusFactory& outputBusFactory = OutputBusFactory()) :
		Subsystem(owner),
		_addr(addr),
		_inputQueueAutoPtr(),
		_providedInputQueuePtr(&inputQueue),
		_outputBusAutoPtr(outputBusFactory.create()),
		_providedOutputBusPtr(),
		_isListening(isListening),
		_clockTimeout(clockTimeout),
		_receiverThreadAutoPtr(),
		_senderThreadAutoPtr(),
		_socket(),
		_providers(),
		_consumers()
	{}
	//! Constructor with user provided output message bus
	/*!
	  \param owner Pointer to the owner subsystem
	  \param addr Message broker remote address if <tt>isListening = TRUE</tt> or a local address to bind otherwise
	  \param outputBus Reference to the output message bus to provide all received messages to
	  \param isListening Does the receiver thread should listen for incoming connection or connect the remote address otherwise
	  \param clockTimeout Subsystem's clock timeout
	  \param inputQueueFactory Input message queue factory object reference
	*/
	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& addr, MessageBusType& outputBus, bool isListening = false,
			const Timeout& clockTimeout = Timeout::defaultTimeout(), const InputQueueFactory& inputQueueFactory = InputQueueFactory()) :
		Subsystem(owner),
		_addr(addr),
		_inputQueueAutoPtr(inputQueueFactory.create()),
		_providedInputQueuePtr(),
		_outputBusAutoPtr(),
		_providedOutputBusPtr(&outputBus),
		_isListening(isListening),
		_clockTimeout(clockTimeout),
		_receiverThreadAutoPtr(),
		_senderThreadAutoPtr(),
		_socket(),
		_providers(),
		_consumers()
	{}
	//! Constructor with user provided input message queue and output message bus
	/*!
	  \param owner Pointer to the owner subsystem
	  \param addr Message broker remote address if <tt>isListening = TRUE</tt> or a local address to bind otherwise
	  \param inputQueue Reference to the input message queue to fetch messages from
	  \param outputBus Reference to the output message bus to provide all received messages to
	  \param isListening Does the receiver thread should listen for incoming connection or connect the remote address otherwise
	  \param clockTimeout Subsystem's clock timeout
	*/
	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& addr, MessageQueueType& inputQueue,
			MessageBusType& outputBus, bool isListening = false, const Timeout& clockTimeout = Timeout::defaultTimeout()) :
		Subsystem(owner),
		_addr(addr),
		_inputQueueAutoPtr(),
		_providedInputQueuePtr(&inputQueue),
		_outputBusAutoPtr(),
		_providedOutputBusPtr(&outputBus),
		_isListening(isListening),
		_clockTimeout(clockTimeout),
		_receiverThreadAutoPtr(),
		_senderThreadAutoPtr(),
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
	//! Returns peer/bind address
	TcpAddrInfo addr() const
	{
		ReadLocker locker(runtimeParamsRWLock);
		return _addr;
	}
	//! Sets peer/bind address
	/*!
	  Subsystem's restart needed to activate changes.

	  \param newValue New message broker address
	*/
	void setAddr(const TcpAddrInfo& newValue)
	{
		WriteLocker locker(runtimeParamsRWLock);
		_addr = newValue;
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
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message provider not found in connection"));
			return;
		}
		_providers.erase(pos);
	}
	//! Removes all message providers
	/*!
	  Subsystem's restart needed to activate changes.
	*/
	void resetProviders()
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
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message consumer not found in connection"));
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
	//! Returns clock timeout
	Timeout clockTimeout() const
	{
		ReadLocker locker(runtimeParamsRWLock);
		return _clockTimeout;
	}
	//! Sets clock timeout
	/*!
	  Subsystem's restart needed to activate changes.

	  \param newValue New clock timeout
	*/
	void setClockTimeout(const Timeout& newValue)
	{
		WriteLocker locker(runtimeParamsRWLock);
		_clockTimeout = newValue;
	}
	//! Sends a message to message broker
	inline bool sendMessage(const Msg& msg)
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
	//! Receiving messages from the network transport and providing them to subscribed consumers abstract thread class
	class AbstractReceiverThread : public Subsystem::AbstractThread
	{
	public:
		//! Constructor
		/*!
		  \param connection Reference to message broker connection object
		*/
		AbstractReceiverThread(AbstractMessageBrokerConnection& connection) :
			AbstractThread(connection),
			_connection(connection),
			_acceptedSocketAutoPtr()
		{}
	protected:
		//! On connected event handler
		/*!
		  \param socket Reference to the connected socket
		*/
		virtual void onConnected(TcpSocket& socket)
		{}
		//! On disconnected event handler
		/*!
		  \param isConnectionAborted True if the connection was aborted by peer or false if subsystem's stop occured
		*/
		virtual void onDisconnected(bool isConnectionAborted)
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
		//! On connect exception event handler
		/*!
		  \param e Pointer to std::exception instance or 0 if exception is unknown
		*/
		virtual void onConnectException(std::exception * e = 0)
		{}
		//! On receive data from transport exception event handler
		/*!
		  \param e Pointer to std::exception instance or 0 if exception is unknown
		*/
		virtual void onReceiveDataException(std::exception * e = 0)
		{}
		//! Receiving message from transport abstract method
		/*!
		  \param socket Socket to read data from
		  \param timeout Data read timeout
		  \return Auto-pointer to the received message or to 0 if no message have been received
		*/
		virtual std::auto_ptr<Msg> receiveMessage(TcpSocket& socket, const Timeout& timeout) = 0;
	private:
		inline TcpSocket& transferSocket()
		{
			if (_connection.isListening()) {
				return &_acceptedSocketAutoPtr.get();
			} else {
				return _connection._socket;
			}
		}
		virtual void run()
		{
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Receiver thread has been started"));
			// Fetching runtime parameters
			std::auto_ptr<TcpAddrInfo> addrAutoPtr;
			ConsumersContainer consumers;
			bool isListening;
			Timeout clockTimeout;
			{
				ReadLocker locker(_connection.runtimeParamsRWLock);
				addrAutoPtr.reset(new TcpAddrInfo(_connection._addr));
				consumers = _connection._consumers;
				isListening = _connection._isListening;
				clockTimeout = _connection._clockTimeout;
			}
			_connection._socket.open();
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Socket has been opened"));
			// Making socket listening if needed
			if (isListening) {
				_connection._socket.bind(*addrAutoPtr.get());
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Socket has been binded"));
				_connection._socket.listen(1);
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Socket has been switched to the listening state"));
			}
			bool connected = false;
			while (true) {
				if (connected) {
					// Has connection
					if (shouldTerminate()) {
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection termination has been detected -> exiting from receiver thread"));
						break;
					}
					// Connection established -> receiving message from the device
					TcpSocket& transferSocket = isListening ? *_acceptedSocketAutoPtr.get() : _connection._socket;
					std::auto_ptr<Msg> msgAutoPtr;
					try {
						msgAutoPtr = receiveMessage(transferSocket, clockTimeout);
					} catch (std::exception& e) {
						onReceiveDataException(&e);
						_connection._senderThreadAutoPtr->resetTransferSocket();
						if (isListening) {
							_acceptedSocketAutoPtr->close();
							_acceptedSocketAutoPtr.reset();
						} else {
							_connection._socket.close();
							_connection._socket.open();
						}
						connected = false;
						onDisconnected(true);
					} catch (...) {
						onReceiveDataException();
						_connection._senderThreadAutoPtr->resetTransferSocket();
						if (isListening) {
							_acceptedSocketAutoPtr->close();
							_acceptedSocketAutoPtr.reset();
						} else {
							_connection._socket.close();
							_connection._socket.open();
						}
						connected = false;
						onDisconnected(true);
					}
					if (msgAutoPtr.get()) {
						// Calling on receive message event callback
						if (!onReceiveMessage(*msgAutoPtr.get())) {
							debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by the on receive event handler"));
							continue;
						}
						// Providing message to the internal output message bus
						if (_connection.outputBus().push(*msgAutoPtr.get())) {
							onProvideMessage(*msgAutoPtr.get(), _connection.outputBus());
						}
						// Providing message to all consumers
						for (typename ConsumersContainer::iterator i = consumers.begin(); i != consumers.end(); ++i) {
							if ((*i)->push(*msgAutoPtr.get())) {
								onProvideMessage(*msgAutoPtr.get(), **i);
							}
						}
					}
				} else {
					// No connection -> establishing a connection
					try {
						if (isListening) {
							_acceptedSocketAutoPtr = _connection._socket.accept(clockTimeout);
							if (!_acceptedSocketAutoPtr.get()) {
								throw Exception(Error(SOURCE_LOCATION_ARGS, "Accepting TCP-connection error"));
							}
							onConnected(*_acceptedSocketAutoPtr.get());
							_connection._senderThreadAutoPtr->setTransferSocket(*_acceptedSocketAutoPtr.get());
						} else {
							_connection._socket.connect(*addrAutoPtr.get());
							onConnected(_connection._socket);
							_connection._senderThreadAutoPtr->setTransferSocket(_connection._socket);
						}
						connected = true;
					} catch (std::exception& e) {
						onConnectException(&e);
					} catch (...) {
						onConnectException();
					}
					//! Waiting for subsystem's termination if no connection established
					if (!connected) {
						if (awaitShouldTerminate(clockTimeout)) {
							debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection termination has been detected -> exiting from receiver thread"));
							break;
						}
					}
				}
			}
			if (connected) {
				if (_acceptedSocketAutoPtr.get()) {
					_acceptedSocketAutoPtr->close();
					_acceptedSocketAutoPtr.reset();
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Accepted socket has been closed"));
				}
				_connection._socket.close();
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Socket has been closed"));
				onDisconnected(false);
			}
		}

		AbstractMessageBrokerConnection& _connection;
		std::auto_ptr<TcpSocket> _acceptedSocketAutoPtr;
	};
	//! Consuming messages from message providers subscribed to and sending them to the network transport abstract thread class
	class AbstractSenderThread : public Subsystem::AbstractThread
	{
	public:
		//! Constructor
		/*!
		  \param connection Reference to message broker connection object
		*/
		AbstractSenderThread(AbstractMessageBrokerConnection& connection) :
			AbstractThread(connection),
			_connection(connection),
			_tranferSocketPtr(0),
			_tranferSocketCond(),
			_consumeBuffer()
		{}
	protected:
		//! On connected event handler
		/*!
		  \param socket Reference to the connected socket
		*/
		virtual void onConnected(TcpSocket& socket)
		{}
		//! On disconnected event handler
		/*!
		  \param isConnectionAborted TRUE if the connection was aborted by peer or false if subsystem's stop occured
		*/
		virtual void onDisconnected(bool isConnectionAborted)
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
		//! Sending message to transport abstract method
		/*!
		  \param msg Constant reference to message to send
		  \param socket Socket to send data to
		  \param timeout Data send timeout
		  \return True if the message has been sent
		*/
		virtual bool sendMessage(const Msg& msg, TcpSocket& socket, const Timeout& timeout) = 0;
	private:
		inline void setTransferSocket(TcpSocket& transferSocket)
		{
			MutexLocker locker(_tranferSocketCond.mutex());
			_tranferSocketPtr = &transferSocket;
			_tranferSocketCond.wakeOne();
		}
		inline void resetTransferSocket()
		{
			MutexLocker locker(_tranferSocketCond.mutex());
			_tranferSocketPtr = 0;
		}
		virtual void run()
		{
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Sender thread has been started"));
			std::auto_ptr<Msg> currentMessageAutoPtr;
			bool sendingMessage = false;
			// Fetching runtime parameters
			ProvidersContainer providers;
			Timeout clockTimeout;
			{
				ReadLocker locker(_connection.runtimeParamsRWLock);
				providers = _connection._providers;
				clockTimeout = _connection._clockTimeout;
			}
			// Susbcribing input message queue to the providers
			typename MessageProviderType::SubscriberListReleaser subscriberListReleaser;
			for (typename ProvidersContainer::iterator i = providers.begin(); i != providers.end(); ++i) {
				std::auto_ptr<typename MessageProviderType::Subscriber> subscriberAutoPtr(new typename MessageProviderType::Subscriber(**i, _connection.inputQueue()));
				subscriberListReleaser.addSubscriber(subscriberAutoPtr.get());
				subscriberAutoPtr.release();
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Sender thread's input queue has been subscribed to the message provider"));
			}
			bool connected = false;
			while (true) {
				if (shouldTerminate()) {
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection termination has been detected -> exiting from sender thread"));
					break;
				}
				if (sendingMessage) {
					bool sendDataExceptionOccured = false;
					{
						MutexLocker locker(_tranferSocketCond.mutex());
						if (_tranferSocketPtr) {
							// Has connection
							if (!connected) {
								// Connection established -> call event handler
								connected = true;
								onConnected(*_tranferSocketPtr);
							}
							bool messageSent = false;
							try {
								messageSent = sendMessage(*currentMessageAutoPtr.get(), *_tranferSocketPtr, clockTimeout);
							} catch (std::exception& e) {
								sendDataExceptionOccured = true;
								onSendDataException(&e);
							} catch (...) {
								sendDataExceptionOccured = true;
								onSendDataException();
							}
							if (messageSent) {
								sendingMessage = false;
								onSendMessage(*currentMessageAutoPtr.get());
							}
						} else {
							// No connection
							if (connected) {
								// Connection has been dropped -> call event handler
								connected = false;
								onDisconnected(true);
							}
							// Waiting for connection to be established in the receiver thread
							_tranferSocketCond.wait(clockTimeout);
						}
					}
					if (sendDataExceptionOccured) {
						// Wait a moment if send data exception has been occured
						// TODO Remove it?
						if (awaitShouldTerminate(clockTimeout)) {
							debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection termination has been detected -> exiting from timer thread"));
							break;
						}
					}
				} else if (_consumeBuffer.empty()) {
					// Fetching all messages from the input to the consume buffer
					size_t consumedMessagesAmount = _connection.inputQueue().popAll(_consumeBuffer, clockTimeout);
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
				onDisconnected(false);
			}
		}

		AbstractMessageBrokerConnection& _connection;
		TcpSocket * _tranferSocketPtr;
		WaitCondition _tranferSocketCond;
		MessageBufferType _consumeBuffer;

		friend class AbstractReceiverThread;
	};
	//! Before subsystem's start event handler
	virtual void beforeStart()
	{
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Opening a socket"));
		if (!_receiverThreadAutoPtr.get()) {
			_receiverThreadAutoPtr.reset(createReceiverThread());
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Receiver thread has been created"));
		}
		if (!_senderThreadAutoPtr.get()) {
			_senderThreadAutoPtr.reset(createSenderThread());
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Sender thread has been created"));
		}
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Starting message broker connection"));
	}
	//! After subsystem's start event handler
	virtual void afterStart()
	{
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection has been started"));
	}
	//! Before subsystem's stop event handler
	virtual void beforeStop()
	{
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Stopping message broker connection"));
	}
	//! After subsystem's stop event handler
	virtual void afterStop()
	{
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection has been stopped"));
	}

	//! Receiver thread creation abstract factory method
	/*!
	  \return Auto-pointer to the new receiver thread object
	*/
	virtual AbstractReceiverThread * createReceiverThread() = 0;
	//! Sender thread creation abstract factory method
	/*!
	  \return Auto-pointer to the new sender thread object
	*/
	virtual AbstractSenderThread * createSenderThread() = 0;
private:
	AbstractMessageBrokerConnection();
	AbstractMessageBrokerConnection(const AbstractMessageBrokerConnection&);						// No copy

	AbstractMessageBrokerConnection& operator=(const AbstractMessageBrokerConnection&);					// No copy

	typedef std::list<MessageProviderType *> ProvidersContainer;
	typedef std::list<AbstractMessageConsumerType *> ConsumersContainer;

	TcpAddrInfo _addr;
	std::auto_ptr<MessageQueueType> _inputQueueAutoPtr;
	MessageQueueType * _providedInputQueuePtr;
	std::auto_ptr<MessageBusType> _outputBusAutoPtr;
	MessageBusType * _providedOutputBusPtr;
	bool _isListening;
	const Timeout _clockTimeout;
	std::auto_ptr<AbstractReceiverThread> _receiverThreadAutoPtr;
	std::auto_ptr<AbstractSenderThread> _senderThreadAutoPtr;
	TcpSocket _socket;
	ProvidersContainer _providers;
	ConsumersContainer _consumers;
};

} // namespace isl

#endif

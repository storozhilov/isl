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
	  \param serverAddrInfo Message broker address
	  \param listeningInputQueueTimeout Listening input queue timeout
	  \param awaitingConnectionTimeout Awaiting connection timeout
	  \param inputQueueFactory Input message queue factory object reference
	  \param outputBusFactory Output message bus factory object reference
	*/
	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& serverAddrInfo,
			const Timeout& listeningInputQueueTimeout = Timeout(0, 100), const Timeout& awaitingConnectionTimeout = Timeout(1),
			const InputQueueFactory& inputQueueFactory = InputQueueFactory(), const OutputBusFactory& outputBusFactory = OutputBusFactory()) :
		Subsystem(owner),
		_serverAddrInfo(serverAddrInfo),
		_inputQueueAutoPtr(inputQueueFactory.create()),
		_providedInputQueuePtr(),
		_outputBusAutoPtr(outputBusFactory.create()),
		_providedOutputBusPtr(),
		_listeningInputQueueTimeout(listeningInputQueueTimeout),
		_awaitingConnectionTimeout(awaitingConnectionTimeout),
		_receiverThreadAutoPtr(),
		_senderThreadAutoPtr(),
		_socket(),
		_providers(),
		_consumers()
	{}
	//! Constructor with user provided input message queue
	/*!
	  \param owner Pointer to the owner subsystem
	  \param serverAddrInfo Message broker address
	  \param inputQueue Reference to the input message queue to fetch messages from
	  \param listeningInputQueueTimeout Listening input queue timeout
	  \param awaitingConnectionTimeout Awaiting connection timeout
	  \param outputBusFactory Output message bus factory object reference
	*/
	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& serverAddrInfo, MessageQueueType& inputQueue,
			const Timeout& listeningInputQueueTimeout = Timeout(0, 100), const Timeout& awaitingConnectionTimeout = Timeout(1),
			const OutputBusFactory& outputBusFactory = OutputBusFactory()) :
		Subsystem(owner),
		_serverAddrInfo(serverAddrInfo),
		_inputQueueAutoPtr(),
		_providedInputQueuePtr(&inputQueue),
		_outputBusAutoPtr(outputBusFactory.create()),
		_providedOutputBusPtr(),
		_listeningInputQueueTimeout(listeningInputQueueTimeout),
		_awaitingConnectionTimeout(awaitingConnectionTimeout),
		_receiverThreadAutoPtr(),
		_senderThreadAutoPtr(),
		_socket(),
		_providers(),
		_consumers()
	{}
	//! Constructor with user provided output message bus
	/*!
	  \param owner Pointer to the owner subsystem
	  \param serverAddrInfo Message broker address
	  \param outputBus Reference to the output message bus to provide all received messages to
	  \param listeningInputQueueTimeout Listening input queue timeout
	  \param awaitingConnectionTimeout Awaiting connection timeout
	  \param inputQueueFactory Input message queue factory object reference
	*/
	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& serverAddrInfo, MessageBusType& outputBus,
			const Timeout& listeningInputQueueTimeout = Timeout(0, 100), const Timeout& awaitingConnectionTimeout = Timeout(1),
			const InputQueueFactory& inputQueueFactory = InputQueueFactory()) :
		Subsystem(owner),
		_serverAddrInfo(serverAddrInfo),
		_inputQueueAutoPtr(inputQueueFactory.create()),
		_providedInputQueuePtr(),
		_outputBusAutoPtr(),
		_providedOutputBusPtr(&outputBus),
		_listeningInputQueueTimeout(listeningInputQueueTimeout),
		_awaitingConnectionTimeout(awaitingConnectionTimeout),
		_receiverThreadAutoPtr(),
		_senderThreadAutoPtr(),
		_socket(),
		_providers(),
		_consumers()
	{}
	//! Constructor with user provided input message queue and output message bus
	/*!
	  \param owner Pointer to the owner subsystem
	  \param serverAddrInfo Message broker address
	  \param inputQueue Reference to the input message queue to fetch messages from
	  \param outputBus Reference to the output message bus to provide all received messages to
	  \param listeningInputQueueTimeout Listening input queue timeout
	  \param awaitingConnectionTimeout Awaiting connection timeout
	*/
	AbstractMessageBrokerConnection(Subsystem * owner, const TcpAddrInfo& serverAddrInfo, MessageQueueType& inputQueue,
			MessageBusType& outputBus, const Timeout& listeningInputQueueTimeout = Timeout(0, 100),
			const Timeout& awaitingConnectionTimeout = Timeout(1)) :
		Subsystem(owner),
		_serverAddrInfo(serverAddrInfo),
		_inputQueueAutoPtr(),
		_providedInputQueuePtr(&inputQueue),
		_outputBusAutoPtr(),
		_providedOutputBusPtr(&outputBus),
		_listeningInputQueueTimeout(listeningInputQueueTimeout),
		_awaitingConnectionTimeout(awaitingConnectionTimeout),
		_receiverThreadAutoPtr(),
		_senderThreadAutoPtr(),
		_socket(),
		_providers(),
		_consumers()
	{}
	//! Returns a reference to the socket connection
	inline TcpSocket& socket()
	{
		return _socket;
	}
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
	//! Adds message provider to subscribe input queue to while running
	/*!
	  \param provider Reference to provider to add
	*/
	void addProvider(MessageProviderType& provider)
	{
		WriteLocker locker(runtimeParamsRWLock);
		_providers.push_back(&provider);
	}
	//! Removes message provider
	/*!
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
	void resetProviders()
	{
		WriteLocker locker(runtimeParamsRWLock);
		_providers.clear();
	}
	//! Adds message consumer for providing incoming messages to while running
	/*!
	  \param consumer Reference to consumer to add
	*/
	void addConsumer(AbstractMessageConsumerType& consumer)
	{
		WriteLocker locker(runtimeParamsRWLock);
		_consumers.push_back(&consumer);
	}
	//! Removes message consumer
	/*!
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
	void resetConsumers()
	{
		WriteLocker locker(runtimeParamsRWLock);
		_consumers.clear();
	}
	//! Sets message broker address
	/*!
	  \param newValue New message broker address
	*/
	void setServerAddr(const TcpAddrInfo& newValue)
	{
		WriteLocker locker(runtimeParamsRWLock);
		_serverAddrInfo = newValue;
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
			_sleepCond()
		{}
		//! Returning a reference to the network connection socket helper method
		inline TcpSocket& socket()
		{
			return _connection._socket;
		}
	protected:
		//! On connected event handler
		/*!
		  Default implementation records an entry to the ISL's debug log
		*/
		virtual void onConnected()
		{
			std::ostringstream msg;
			msg << "TCP-connection to " << _connection._serverAddrInfo.firstEndpoint().host << ':' <<
				_connection._serverAddrInfo.firstEndpoint().port << " has been successfully established";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		}
		//! On disconnected event handler
		/*!
		  Default implementation records an entry to the ISL's debug log

		  \param isConnectionAborted True if the connection was aborted by peer or false if subsystem's stop occured
		*/
		virtual void onDisconnected(bool isConnectionAborted)
		{
			std::ostringstream msg;
			msg << "TCP-connection to " << _connection._serverAddrInfo.firstEndpoint().host << ':' <<
				_connection._serverAddrInfo.firstEndpoint().port << " server " << (isConnectionAborted ? "has been aborted" : "has been closed");
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		}
		//! On receive message from transport event handler
		/*!
		  Default implementation records an entry to the ISL's debug log and returns true

		  \param msg Constant reference to the received message
		  \return True if to proceed with the message or false to discard it
		*/
		virtual bool onReceiveMessage(const Msg& msg)
		{
			std::ostringstream oss;
			oss << "Message has been received from " << _connection._serverAddrInfo.firstEndpoint().host << ':' <<
				_connection._serverAddrInfo.firstEndpoint().port << " server";
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
			if (&consumer == &_connection.outputBus()) {
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been provided to the internal output bus"));
			} else {
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been provided to the consumer"));
			}
		}
		//! On connect exception event handler
		/*!
		  Default implementation records an entry to the ISL's error log

		  \param e Pointer to std::exception instance or 0 if exception is unknown
		*/
		virtual void onConnectException(std::exception * e = 0)
		{
			std::ostringstream msg;
			msg << "Connecting to " << _connection._serverAddrInfo.firstEndpoint().host << ':' <<
				_connection._serverAddrInfo.firstEndpoint().port << " server ";
			if (e) {
				msg << "error";
				errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, *e, msg.str()));
			} else {
				msg << "unknown error";
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
		}
		//! On receive data from transport exception event handler
		/*!
		  Default implementation records an entry to the ISL's error log

		  \param e Pointer to std::exception instance or 0 if exception is unknown
		*/
		virtual void onReceiveDataException(std::exception * e = 0)
		{
			std::ostringstream msg;
			msg << "Receiving data from " << _connection._serverAddrInfo.firstEndpoint().host << ':' <<
				_connection._serverAddrInfo.firstEndpoint().port << " server ";
			if (e) {
				msg << "error -> reestablishing connection";
				errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, *e, msg.str()));
			} else {
				msg << "unknown error -> reestablishing connection";
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
		}
		//! Receiving message from transport abstract method
		/*!
		  \return Auto-pointer to the received message or to 0 if no message have been received
		*/
		virtual std::auto_ptr<Msg> receiveMessage() = 0;
	private:
		virtual void run()
		{
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Receiver thread has been started"));
			// Fetching consumers to provide incoming messages to
			ConsumersContainer consumers;
			{
				ReadLocker locker(_connection.runtimeParamsRWLock);
				consumers = _connection._consumers;
			}
			_connection._socket.open();
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Socket has been opened"));
			while (true) {
				if (shouldTerminate()) {
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection termination has been detected -> exiting from receiver thread"));
					break;
				}
				if (_connection._socket.connected()) {
					// Receiving message from the device
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
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Socket has been closed"));
				onDisconnected(false);
			}
		}

		AbstractMessageBrokerConnection& _connection;
		WaitCondition _sleepCond;
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
			_sleepCond(),
			_consumeBuffer()
		{}
		//! Returning a reference to the network connection socket helper method
		inline TcpSocket& socket()
		{
			return _connection._socket;
		}
	protected:
		//! On consume message from any provider event handler
		/*!
		  Default implementation records an entry to the ISL's debug log and returns true

		  \param msg Constant reference to the consumed message
		  \return True if to proceed with the message or false to discard it
		*/
		virtual bool onConsumeMessage(const Msg& msg)
		{
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been consumed from the input message queue"));
			return true;
		}
		//! On send message to transport event handler
		/*!
		  Default implementation records an entry to the ISL's debug log

		  \param msg Constant reference to the message has been sent
		*/
		virtual void onSendMessage(const Msg& msg)
		{
			std::ostringstream oss;
			oss << "Message has been sent to " << _connection._serverAddrInfo.firstEndpoint().host << ':'
				<< _connection._serverAddrInfo.firstEndpoint().port << " server";
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
			msg << "Sending data to " << _connection._serverAddrInfo.firstEndpoint().host << ':' <<
				_connection._serverAddrInfo.firstEndpoint().port << " server ";
			if (e) {
				msg << "error";
				errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, *e, msg.str()));
			} else {
				msg << "unknown error";
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
		virtual void run()
		{
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Sender thread has been started"));
			std::auto_ptr<Msg> currentMessageAutoPtr;
			bool sendingMessage = false;
			// Fetching providers to subscribe to
			ProvidersContainer providers;
			{
				ReadLocker locker(_connection.runtimeParamsRWLock);
				providers = _connection._providers;
			}
			// Susbcribing input message queue to the providers
			typename MessageProviderType::SubscriberListReleaser subscriberListReleaser;
			for (typename ProvidersContainer::iterator i = providers.begin(); i != providers.end(); ++i) {
				std::auto_ptr<typename MessageProviderType::Subscriber> subscriberAutoPtr(new typename MessageProviderType::Subscriber(**i, _connection.inputQueue()));
				subscriberListReleaser.addSubscriber(subscriberAutoPtr.get());
				subscriberAutoPtr.release();
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Sender thread's input queue has been subscribed to the message provider"));
			}
			while (true) {
				if (shouldTerminate()) {
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message broker connection termination has been detected -> exiting from sender thread"));
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
							onSendMessage(*currentMessageAutoPtr.get());
						}
					} else {
						// Waiting for socket to be connected
						MutexLocker locker(_sleepCond.mutex());
						_sleepCond.wait(_connection._awaitingConnectionTimeout);
					}
				} else if (_consumeBuffer.empty()) {
					// Fetching all messages from the input to the consume buffer
					size_t consumedMessagesAmount = _connection.inputQueue().popAll(_consumeBuffer, _connection._listeningInputQueueTimeout);
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

		AbstractMessageBrokerConnection& _connection;
		WaitCondition _sleepCond;
		MessageBufferType _consumeBuffer;
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

	TcpAddrInfo _serverAddrInfo;
	std::auto_ptr<MessageQueueType> _inputQueueAutoPtr;
	MessageQueueType * _providedInputQueuePtr;
	std::auto_ptr<MessageBusType> _outputBusAutoPtr;
	MessageBusType * _providedOutputBusPtr;
	const Timeout _listeningInputQueueTimeout;
	const Timeout _awaitingConnectionTimeout;
	std::auto_ptr<AbstractReceiverThread> _receiverThreadAutoPtr;
	std::auto_ptr<AbstractSenderThread> _senderThreadAutoPtr;
	TcpSocket _socket;
	ProvidersContainer _providers;
	ConsumersContainer _consumers;
};

} // namespace isl

#endif

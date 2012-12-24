#include <isl/PidFile.hxx>
#include <isl/Server.hxx>
#include <isl/FileLogTarget.hxx>
#include <isl/AbstractMessageBrokerService.hxx>
#include <isl/AbstractMessageBrokerConnection.hxx>
#include <isl/AbstractMessageBrokerListeningConnection.hxx>

#define MAX_CLIENTS 10
#define SERVICE_LISTEN_PORT 8888
#define CONNECTION_LISTEN_PORT 8889
#define CONNECT_PORT 8890

typedef std::string Message;

Message * parseMessage(std::string& buffer)
{
	if (buffer.empty()) {
		return 0;
	}
	size_t crlfPos = buffer.find("\r\n");
	if (crlfPos != std::string::npos) {
		Message * msgPtr = new Message(buffer.substr(0, crlfPos));
		buffer = buffer.substr(crlfPos + 2);
		return msgPtr;
	}
	size_t lfPos = buffer.find("\n");
	if (lfPos == std::string::npos) {
		return 0;
	}
	Message * msgPtr = new Message(buffer.substr(0, lfPos));
	buffer = buffer.substr(lfPos + 1);
	return msgPtr;
}

class MessageBrokerService : public isl::AbstractMessageBrokerService<Message>
{
public:
	MessageBrokerService(isl::Subsystem * owner, size_t maxClients) :
		isl::AbstractMessageBrokerService<Message>(owner, maxClients)
	{}
private:
	class Task : public AbstractTask
	{
	public:
		Task(MessageBrokerService& service, isl::TcpSocket& socket) :
			AbstractTask(service, socket),
			_receiveBuffer(),
			_sendBuffer(),
			_bytesSent(0)
		{}
	private:
		virtual void beforeExecuteReceive()
		{
			inputQueue().push(MessageType("Hello from broadcast message broker service! Type \"bye\" to close session."));
		}
		virtual bool onReceiveMessage(const MessageType& msg)
		{
			if (msg == "bye") {
				appointTermination();
				return false;
			} else {
				return true;
			}
		}
		virtual MessageType * receiveMessage(const isl::Timeout& timeout)
		{
			MessageType * msgPtr = parseMessage(_receiveBuffer);
			if (msgPtr) {
				return msgPtr;
			}
			char buf[4096];
			size_t bytesReceived = socket().read(buf, sizeof(buf), timeout);
			if (bytesReceived <= 0) {
				return 0;
			}
			_receiveBuffer.append(buf, bytesReceived);
			return parseMessage(_receiveBuffer);
		}
		virtual bool sendMessage(const MessageType& msg, const isl::Timeout& timeout)
		{
			if (_sendBuffer.empty()) {
				_sendBuffer = msg;
				_sendBuffer += "\r\n";
			}
			size_t curBytesSent = socket().write(_sendBuffer.c_str() + _bytesSent, _sendBuffer.size() - _bytesSent, timeout);
			_bytesSent += curBytesSent;
			if (_bytesSent >= _sendBuffer.size()) {
				_sendBuffer.clear();
				_bytesSent = 0;
				return true;
			} else {
				return false;
			}
		}

		std::string _receiveBuffer;
		std::string _sendBuffer;
		size_t _bytesSent;
	};

	virtual AbstractTask * createTask(isl::TcpSocket& socket)
	{
		return new Task(*this, socket);
	}
};

class MessageBrokerConnection : public isl::AbstractMessageBrokerConnection<Message>
{
public:
	MessageBrokerConnection(isl::Subsystem * owner, const isl::TcpAddrInfo& remoteAddr) :
		isl::AbstractMessageBrokerConnection<Message>(owner, remoteAddr),
		_receiveBuffer(),
		_sendBuffer(),
		_bytesSent(0)
	{}
private:
	virtual void onReceiverConnected(isl::TcpSocket& socket)
	{
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Connection established in the receiver thread"));
	}
	virtual void onReceiverDisconnected(bool isConnectionAborted)
	{
		if (isConnectionAborted) {
			isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Connection aborted in the receiver thread"));
		} else {
			isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Connection closed in the receiver thread"));
		}
	}
	/*virtual void onConnectFailed(const isl::Exception& e, size_t failedAttempts)
	{
		isl::errorLog().log(isl::ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Establishing connection error occured in the receiver thread"));
	}*/
	virtual void onSenderConnected(isl::TcpSocket& socket)
	{
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Connection established in the sender thread"));
	}
	virtual void onSenderDisconnected(bool isConnectionAborted)
	{
		if (isConnectionAborted) {
			isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Connection aborted in the sender thread"));
		} else {
			isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Connection closed in the sender thread"));
		}
	}

	virtual MessageType * receiveMessage(isl::TcpSocket& socket, const isl::Timeout& timeout)
	{
		MessageType * msgPtr = parseMessage(_receiveBuffer);
		if (msgPtr) {
			return msgPtr;
		}
		char buf[4096];
		size_t bytesReceived = socket.read(buf, sizeof(buf), timeout);
		if (bytesReceived <= 0) {
			return 0;
		}
		_receiveBuffer.append(buf, bytesReceived);
		return parseMessage(_receiveBuffer);
	}
	virtual bool sendMessage(const MessageType& msg, isl::TcpSocket& socket, const isl::Timeout& timeout)
	{
		if (_sendBuffer.empty()) {
			_sendBuffer = msg;
			_sendBuffer += "\r\n";
		}
		size_t curBytesSent = socket.write(_sendBuffer.c_str() + _bytesSent, _sendBuffer.size() - _bytesSent, timeout);
		_bytesSent += curBytesSent;
		if (_bytesSent >= _sendBuffer.size()) {
			_sendBuffer.clear();
			_bytesSent = 0;
			return true;
		} else {
			return false;
		}
	}

	std::string _receiveBuffer;
	std::string _sendBuffer;
	size_t _bytesSent;
};

class MessageBrokerListeningConnection : public isl::AbstractMessageBrokerListeningConnection<Message>
{
public:
	MessageBrokerListeningConnection(isl::Subsystem * owner, const isl::TcpAddrInfo& localAddr) :
		isl::AbstractMessageBrokerListeningConnection<Message>(owner, localAddr),
		_receiveBuffer(),
		_sendBuffer(),
		_bytesSent(0)
	{}
private:
	virtual void onReceiverConnected(isl::TcpSocket& socket)
	{
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Connection established in the receiver thread"));
	}
	virtual void onReceiverDisconnected(bool isConnectionAborted)
	{
		if (isConnectionAborted) {
			isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Connection aborted in the receiver thread"));
		} else {
			isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Connection closed in the receiver thread"));
		}
	}
	/*virtual void onAcceptFailed(size_t failedAttempts)
	{
		isl::errorLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Accepting connection timeout expired in the receiver thread"));
	}*/
	virtual void onSenderConnected(isl::TcpSocket& socket)
	{
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Connection established in the sender thread"));
	}
	virtual void onSenderDisconnected(bool isConnectionAborted)
	{
		if (isConnectionAborted) {
			isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Connection aborted in the sender thread"));
		} else {
			isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Connection closed in the sender thread"));
		}
	}

	virtual MessageType * receiveMessage(isl::TcpSocket& socket, const isl::Timeout& timeout)
	{
		MessageType * msgPtr = parseMessage(_receiveBuffer);
		if (msgPtr) {
			return msgPtr;
		}
		char buf[4096];
		size_t bytesReceived = socket.read(buf, sizeof(buf), timeout);
		if (bytesReceived <= 0) {
			return 0;
		}
		_receiveBuffer.append(buf, bytesReceived);
		return parseMessage(_receiveBuffer);
	}
	virtual bool sendMessage(const MessageType& msg, isl::TcpSocket& socket, const isl::Timeout& timeout)
	{
		if (_sendBuffer.empty()) {
			_sendBuffer = msg;
			_sendBuffer += "\r\n";
		}
		size_t curBytesSent = socket.write(_sendBuffer.c_str() + _bytesSent, _sendBuffer.size() - _bytesSent, timeout);
		_bytesSent += curBytesSent;
		if (_bytesSent >= _sendBuffer.size()) {
			_sendBuffer.clear();
			_bytesSent = 0;
			return true;
		} else {
			return false;
		}
	}

	std::string _receiveBuffer;
	std::string _sendBuffer;
	size_t _bytesSent;
};

// Our broadcast message broker server class
class BroadcastMessageBrokerServer : public isl::Server
{
public:
	BroadcastMessageBrokerServer(int argc, char * argv[]) :
		isl::Server(argc, argv),
		_service(this, MAX_CLIENTS),
		_connection(this, isl::TcpAddrInfo(isl::TcpAddrInfo::IpV4, isl::TcpAddrInfo::LoopbackAddress, CONNECT_PORT)),
		_listeningConnection(this, isl::TcpAddrInfo(isl::TcpAddrInfo::IpV4, isl::TcpAddrInfo::WildcardAddress, CONNECTION_LISTEN_PORT)),
		_messageBus()
	{
		_service.addListener(isl::TcpAddrInfo(isl::TcpAddrInfo::IpV4, isl::TcpAddrInfo::WildcardAddress, SERVICE_LISTEN_PORT));
		_service.addProvider(_messageBus);
		_service.addConsumer(_messageBus);
		_connection.addProvider(_messageBus);
		_connection.addConsumer(_messageBus);
		_listeningConnection.addProvider(_messageBus);
		_listeningConnection.addConsumer(_messageBus);
	}
private:
	BroadcastMessageBrokerServer();
	BroadcastMessageBrokerServer(const BroadcastMessageBrokerServer&);

	MessageBrokerService _service;
	MessageBrokerConnection _connection;
	MessageBrokerListeningConnection _listeningConnection;
	isl::MessageBus<Message> _messageBus;
};

int main(int argc, char *argv[])
{
	isl::PidFile pidFile("bmb.pid");					// Writing PID of the server to file
	isl::debugLog().connectTarget(isl::FileLogTarget("bmb.log"));		// Connecting basic logs to one file target
	isl::warningLog().connectTarget(isl::FileLogTarget("bmb.log"));
	isl::errorLog().connectTarget(isl::FileLogTarget("bmb.log"));
	BroadcastMessageBrokerServer server(argc, argv);			// Creating server object
	server.run();								// Running server
	isl::debugLog().disconnectTargets();					// Disconnecting basic logs from the targets
	isl::warningLog().disconnectTargets();
	isl::errorLog().disconnectTargets();
}

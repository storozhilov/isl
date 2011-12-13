#include "EchoMessageBroker.hxx"

EchoMessageBroker::EchoMessageBroker(AbstractSubsystem * owner) :
	isl::AbstractMessageBroker(owner, 8081, 5)
{}

EchoMessageBroker::AbstractMessage * EchoMessageBroker::recieveMessage(isl::TcpSocket& socket, ReceiverTask& recieverTask)
{
	char currentCharacter;
	if (!socket.getChar(currentCharacter, timeout())) {
		return 0;
	}
	// Transmission has been started
	std::string msgData(1, currentCharacter);
	while (currentCharacter != '\n') {
		if (!socket.getChar(currentCharacter, timeout())) {
			throw std::runtime_error("Timeout has been expired while receiving message from the client");
		}
		msgData += currentCharacter;
	}
	return new Message(msgData.c_str());
}

void EchoMessageBroker::processMessage(const EchoMessageBroker::AbstractMessage& message, EchoMessageBroker::ReceiverTask& recieverTask, SenderTask& senderTask)
{
	std::auto_ptr<AbstractMessage> outMessageAutoPtr(message.clone());
	if (senderTask.sendMessage(outMessageAutoPtr.get())) {
		outMessageAutoPtr.release();
	}
	const Message& inMessage = dynamic_cast<const Message&>(message);
	// Terminate client if it wants to
	if (std::string(inMessage.data(), inMessage.size()) == "bye\r\n") {
		recieverTask.terminate();
	}
}

void EchoMessageBroker::sendMessage(isl::TcpSocket& socket, const EchoMessageBroker::AbstractMessage& message, SenderTask& senderTask)
{
	const Message& msg = dynamic_cast<const Message&>(message);
	try {
		unsigned int totalBytesSent = 0;
		while (totalBytesSent < msg.size()) {
			unsigned int bytesSent = socket.write(msg.data() + totalBytesSent, msg.size() - totalBytesSent, timeout());
			if (bytesSent <= 0) {
				isl::Core::warningLog.log(isl::DebugLogMessage(SOURCE_LOCATION_ARGS, L"Timeout expired while sending the message"));
			}
			totalBytesSent += bytesSent;
		}
		isl::Core::debugLog.log(isl::DebugLogMessage(SOURCE_LOCATION_ARGS, "Message has been successfully sent"));
	} catch (std::exception& e) {
		isl::Core::errorLog.log(isl::ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Sending message error - exiting from the task execution"));
		return;
	} catch (...) {
		isl::Core::errorLog.log(isl::DebugLogMessage(SOURCE_LOCATION_ARGS, L"Sending message unknown error - exiting from the task execution"));
		return;
	}
}

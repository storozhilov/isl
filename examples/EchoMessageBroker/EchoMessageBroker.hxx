#ifndef ISL__EXAMPLES__ECHO_MESSAGE_BROKER__HXX
#define ISL__EXAMPLES__ECHO_MESSAGE_BROKER__HXX

#include <isl/AbstractServer.hxx>
#include <isl/AbstractMessageBroker.hxx>

class EchoMessageBroker : public isl::AbstractMessageBroker
{
public:
	EchoMessageBroker(AbstractSubsystem * owner);
private:
	EchoMessageBroker();
	EchoMessageBroker(const EchoMessageBroker&);

	class Message : public AbstractMessage
	{
	public:
		Message(const char * data) :
			AbstractMessage(),
			_data(data)
		{}
		const char * data() const
		{
			return _data.c_str();
		}
		unsigned int size() const
		{
			return _data.size();
		}

		virtual AbstractMessage * clone() const
		{
			return new Message(*this);
		}
	private:
		std::string _data;
	};

	virtual AbstractMessage * recieveMessage(isl::TcpSocket& socket, ReceiverTask& recieverTask);
	virtual void processMessage(const AbstractMessage& message, ReceiverTask& recieverTask, SenderTask& senderTask);
	virtual void sendMessage(isl::TcpSocket& socket, const AbstractMessage& message, SenderTask& senderTask);
};

#endif


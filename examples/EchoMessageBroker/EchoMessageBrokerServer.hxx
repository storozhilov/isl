#ifndef ISL__EXAMPLES__ECHO_MESSAGE_BROKER_SERVER__HXX
#define ISL__EXAMPLES__ECHO_MESSAGE_BROKER_SERVER__HXX

#include <isl/AbstractServer.hxx>
#include <isl/SignalHandler.hxx>
#include "EchoMessageBroker.hxx"

class EchoMessageBrokerServer : public isl::AbstractServer
{
public:
	EchoMessageBrokerServer(int argc, char * argv[]);
	
	virtual bool start();
	virtual void stop();
	virtual bool restart();
private:
	EchoMessageBrokerServer();
	EchoMessageBrokerServer(const EchoMessageBrokerServer&);

	isl::SignalHandler _signalHandler;
	EchoMessageBroker _messageBroker;
};

#endif


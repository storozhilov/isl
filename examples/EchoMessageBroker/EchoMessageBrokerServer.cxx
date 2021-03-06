#include "EchoMessageBrokerServer.hxx"

EchoMessageBrokerServer::EchoMessageBrokerServer(int argc, char * argv[]) :
	isl::AbstractServer(argc, argv),
	_signalHandler(this),
	_messageBroker(this)
{}

void EchoMessageBrokerServer::start()
{
	setState(IdlingState, StartingState);
	_signalHandler.start();
	_messageBroker.start();
	setState(StartingState, RunningState);
}

void EchoMessageBrokerServer::stop()
{
	setState(StoppingState);
	_signalHandler.stop();
	_messageBroker.stop();
	setState(IdlingState);
}

void EchoMessageBrokerServer::restart()
{
	setState(StoppingState);
	_signalHandler.stop();
	_messageBroker.stop();
	setState(StoppingState, StartingState);
	_signalHandler.start();
	_messageBroker.start();
	setState(StartingState, RunningState);
}


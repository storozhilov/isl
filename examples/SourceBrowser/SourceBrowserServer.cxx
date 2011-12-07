#include "SourceBrowserServer.hxx"

SourceBrowserServer::SourceBrowserServer(int argc, char * argv[]) :
	isl::AbstractServer(argc, argv),
	_signalHandler(*this),
	_sourceBrowserService(this, 8080, 5, L".")
{}

bool SourceBrowserServer::start()
{
	setState(IdlingState, StartingState);
	_signalHandler.start();
	_sourceBrowserService.start();
	setState(StartingState, RunningState);
}

void SourceBrowserServer::stop()
{
	setState(StoppingState);
	_signalHandler.stop();
	_sourceBrowserService.stop();
	setState(IdlingState);
}

bool SourceBrowserServer::restart()
{
	setState(StoppingState);
	_signalHandler.stop();
	_sourceBrowserService.stop();
	setState(StoppingState, StartingState);
	_signalHandler.start();
	_sourceBrowserService.start();
	setState(StartingState, RunningState);
}


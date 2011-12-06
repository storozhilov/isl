#include "SourceBrowserServer.hxx"

SourceBrowserServer::SourceBrowserServer(int argc, char * argv[]) :
	isl::AbstractServer(argc, argv),
	_signalHandler(*this),
	_sourceBrowser(this, 5, 8080, L".")
{}

void SourceBrowserServer::onStart()
{
	_signalHandler.start();
	_sourceBrowser.start();
}

void SourceBrowserServer::onStop()
{
	_signalHandler.stop();
	_sourceBrowser.stop();
}

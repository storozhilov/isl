#ifndef TEST_SERVER__HXX
#define TEST_SERVER__HXX

#include <isl/AbstractServer.hxx>
#include <isl/SignalHandler.hxx>
#include "SourceBrowserService.hxx"

class SourceBrowserServer : public isl::AbstractServer
{
public:
	SourceBrowserServer(int argc, char * argv[]);
	
	virtual void start();
	virtual void stop();
	virtual void restart();
private:
	SourceBrowserServer();
	SourceBrowserServer(const SourceBrowserServer&);

	isl::SignalHandler _signalHandler;
	SourceBrowserService _sourceBrowserService;
};

#endif


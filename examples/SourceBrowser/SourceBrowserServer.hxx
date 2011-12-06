#ifndef TEST_SERVER__HXX
#define TEST_SERVER__HXX

#include <isl/AbstractServer.hxx>
#include <isl/SignalHandler.hxx>
#include "SourceBrowser.hxx"

class SourceBrowserServer : public isl::AbstractServer
{
public:
	SourceBrowserServer(int argc, char * argv[]);
private:
	SourceBrowserServer();
	SourceBrowserServer(const SourceBrowserServer&);
	
	virtual void onStart();
	virtual void onStop();

	isl::exp::SignalHandler _signalHandler;
	isl::SourceBrowser _sourceBrowser;
};

#endif


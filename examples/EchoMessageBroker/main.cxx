#define LIBISL__DEBUGGING_ON 1

#include <isl/FileLogTarget.hxx>
#include <isl/Core.hxx>
#include <isl/AbstractHTTPTask.hxx>
#include "EchoMessageBrokerServer.hxx"

int main(int argc, char *argv[])
{
	//isl::Core::daemonize();
	isl::Core::writePid("embd.pid");
	isl::Core::debugLog.setPrefix(L"DEBUG");
	isl::Core::debugLog.connectTarget(isl::FileLogTarget("server.log"));
	isl::Core::warningLog.setPrefix(L"WARNING");
	isl::Core::warningLog.connectTarget(isl::FileLogTarget("server.log"));
	isl::Core::errorLog.setPrefix(L"ERROR");
	isl::Core::errorLog.connectTarget(isl::FileLogTarget("server.log"));
	EchoMessageBrokerServer server(argc, argv);
	server.run();
	isl::Core::debugLog.disconnectTargets();
	isl::Core::warningLog.disconnectTargets();
	isl::Core::errorLog.disconnectTargets();
	std::cout << "Server stopped" << std::endl;
}

#define LIBISL__DEBUGGING_ON 1

#include <isl/FileLogTarget.hxx>
#include <isl/Core.hxx>
#include <isl/AbstractHTTPTask.hxx>
#include "SourceBrowserServer.hxx"

int main(int argc, char *argv[])
{
	//isl::Core::daemonize();
	isl::Core::writePid("sbd.pid");
	isl::Core::debugLog.setPrefix(L"DEBUG");
	isl::Core::debugLog.connectTarget(isl::FileLogTarget("server.log"));
	isl::Core::warningLog.setPrefix(L"WARNING");
	isl::Core::warningLog.connectTarget(isl::FileLogTarget("server.log"));
	isl::Core::errorLog.setPrefix(L"ERROR");
	isl::Core::errorLog.connectTarget(isl::FileLogTarget("server.log"));
	isl::AbstractHTTPTask::debugLog.connectTarget(isl::FileLogTarget("http.log"));
	isl::AbstractHTTPTask::debugLog.setPrefix(L"DEBUG");
	isl::AbstractHTTPTask::warningLog.connectTarget(isl::FileLogTarget("http.log"));
	isl::AbstractHTTPTask::warningLog.setPrefix(L"WARNING");
	isl::AbstractHTTPTask::errorLog.connectTarget(isl::FileLogTarget("http.log"));
	isl::AbstractHTTPTask::errorLog.setPrefix(L"ERROR");
	isl::AbstractHTTPTask::accessLog.connectTarget(isl::FileLogTarget("access.log"));
	SourceBrowserServer server(argc, argv);
	server.run();
	isl::Core::debugLog.disconnectTargets();
	isl::Core::warningLog.disconnectTargets();
	isl::Core::errorLog.disconnectTargets();
	isl::AbstractHTTPTask::debugLog.disconnectTargets();
	isl::AbstractHTTPTask::warningLog.disconnectTargets();
	isl::AbstractHTTPTask::errorLog.disconnectTargets();
	isl::AbstractHTTPTask::accessLog.disconnectTargets();
	std::cout << "Server stopped" << std::endl;
}

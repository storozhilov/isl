#define LIBISL__DEBUGGING_ON 1

#include <isl/Core.hxx>
//#include <isl/String.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/Exception.hxx>
#include <isl/HttpRequestReader.hxx>
#include <isl/FileLogTarget.hxx>
#include <iostream>
#include <memory>

#define LISTEN_PORT 8080
#define LISTEN_BACKLOG 10
#define ACCEPT_SECONDS_TIMEOUT 1
#define TRANSMISSION_SECONDS_TIMEOUT 60

int main(int argc, char *argv[])
{
	//isl::Core::daemonize();
	isl::Core::writePid("hsd.pid");
	isl::Core::debugLog.setPrefix(L"DEBUG");
	isl::Core::debugLog.connectTarget(isl::FileLogTarget("hsd.log"));
	isl::Core::warningLog.setPrefix(L"WARNING");
	isl::Core::warningLog.connectTarget(isl::FileLogTarget("hsd.log"));
	isl::Core::errorLog.setPrefix(L"ERROR");
	isl::Core::errorLog.connectTarget(isl::FileLogTarget("hsd.log"));
	//SourceBrowserServer server(argc, argv);
	//server.run();
	isl::TcpSocket s;
	s.open();
	s.bind(LISTEN_PORT);
	s.listen(LISTEN_BACKLOG);
	while (true) {
		std::auto_ptr<isl::TcpSocket>ss(s.accept(isl::Timeout(ACCEPT_SECONDS_TIMEOUT)));
		if (!ss.get()) {
			std::cout << "Listen timeout has been expired" << std::endl;
			continue;
		}
		isl::HttpRequestReader request(*ss.get());
		try {
			//isl::HttpRequestReader 
			request.receive();
			std::cout << "HTTP-request has been recieved:" << std::endl <<
				"\tURI: \"" << request.uri() << '"' << std::endl <<
				"\tpath: \"" << isl::String::decodePercent(request.path()) << '"' << std::endl <<
				"\tquery: \"" << request.query() << '"' << std::endl;
			isl::Http::Params get = request.get();
			for (isl::Http::Params::const_iterator i = request.get().begin(); i != request.get().end(); ++i) {
				std::cout << "\tget[\"" << i->first << "\"] = \"" << i->second << '"' << std::endl;
			}
			for (isl::Http::Header::const_iterator i = request.header().begin(); i != request.header().end(); ++i) {
				std::cout << "\theader[\"" << i->first << "\"] = \"" << i->second << '"' << std::endl;
			}
			//for (isl::Http::Params::const_iterator i = request.cookies().begin(); i != request.cookies().end(); ++i) {
			//	std::cout << "\tcookie[\"" << i->first << "\"] = \"" << i->second << '"' << std::endl;
			//}
			for (isl::Http::RequestCookies::const_iterator i = request.cookies().begin(); i != request.cookies().end(); ++i) {
				std::cout << "\tcookie[\"" << i->first << "\"] = \"" << i->second.value << '"' << std::endl;
			}
		} catch (isl::Exception& e) {
			std::cerr << isl::String::utf8Encode(e.debug()) << std::endl;
			return 1;
		} catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
			return 1;
		} catch (...) {
			std::cerr << "Unknown error occured." << std::endl;
			return 1;
		}
	}
	isl::Core::debugLog.disconnectTargets();
	isl::Core::warningLog.disconnectTargets();
	isl::Core::errorLog.disconnectTargets();
}

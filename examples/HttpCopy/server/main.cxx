#define LIBISL__DEBUGGING_ON 1

#include <isl/common.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/Exception.hxx>
#include <isl/Http.hxx>
#include <isl/FileLogTarget.hxx>
#include <isl/HttpRequestStreamReader.hxx>
#include <isl/HttpResponseStreamWriter.hxx>
#include <memory>
#include <iostream>
#include <stdexcept>

#define BUFFER_SIZE 4096
#define LISTEN_PORT 8081
#define LISTEN_BACKLOG 10
#define ACCEPT_SECONDS_TIMEOUT 1
#define TRANSMISSION_SECONDS_TIMEOUT 60

int main(int argc, char *argv[])
{
	isl::debugLog().connectTarget(isl::FileLogTarget("server.log"));;
	isl::TcpSocket s;
	s.open();
	s.bind(isl::TcpAddrInfo(isl::TcpAddrInfo::IpV4, isl::TcpAddrInfo::WildcardAddress, LISTEN_PORT));
	s.listen(LISTEN_BACKLOG);
	while (true) {
		std::auto_ptr<isl::TcpSocket>ss(s.accept(isl::Timeout(ACCEPT_SECONDS_TIMEOUT)));
		if (!ss.get()) {
			std::cout << "Listen timeout has been expired" << std::endl;
			continue;
		}
		isl::HttpRequestStreamReader r(*ss.get());
		unsigned int totalBytesRead = 0;
		char buf[BUFFER_SIZE];
		try {
			while (!r.parser().isCompleted()) {
				//bool timeoutExpired;
				//unsigned int bytesRead = r.read(buf, BUFFER_SIZE, isl::Timeout(TRANSMISSION_SECONDS_TIMEOUT), &timeoutExpired);
				//if (timeoutExpired) {
				//	throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, "Receiving data timeout expired"));
				//}
				size_t rawBytesFetched;
				size_t bytesRead = r.read(buf, BUFFER_SIZE, isl::Timeout(TRANSMISSION_SECONDS_TIMEOUT), &rawBytesFetched);
				if (rawBytesFetched <= 0) {
					throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, "Receiving data timeout expired"));
				}
				if (bytesRead > 0) {
					std::cout.write(buf, bytesRead);
					std::cout.flush();
				}
				totalBytesRead += bytesRead;
			}
			std::cout << "Source filename is \"" << r.uri() << '"' << std::endl;
			std::cout << "Current directory is \"" << isl::Http::paramValue(r.parser().header(), "X-Current-Directory") << '"' << std::endl;
			std::cout << "Target filename/directory is \"" << isl::Http::paramValue(r.parser().header(), "X-Dest-Filename") << '"' << std::endl;
			isl::HttpResponseStreamWriter w(*ss.get());
			w.setHeaderField("X-Copy-Status", "OK");
			/*if (!w.writeBodyless()) {
				while (!w.flush());
			}*/
			w.writeBodyless();
		} catch (isl::Exception& e) {
			std::cerr << e.what() << std::endl;
			std::cerr << "HTTP-request parser state is " << r.parser().state() << std::endl;
			return 1;
		} catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
			std::cerr << "HTTP-request parser state is " << r.parser().state() << std::endl;
			return 1;
		} catch (...) {
			std::cerr << "Unknown error occured." << std::endl;
			std::cerr << "HTTP-request parser state is " << r.parser().state() << std::endl;
			return 1;
		}
		//std::cout << "\nHTTP-request has been succefully read. Method: '" << r.method() << "', URI: '" << r.uri() << "' Version: '" << r.version() << "', Headers:" << std::endl;
		//isl::HttpRequestStreamReader::Header header = r.header();
		//for (isl::HttpRequestStreamReader::Header::const_iterator i = header.begin(); i != header.end(); ++i) {
		//	std::cout << "\t'" << i->first << "': '" << i->second << '\'' << std::endl;
		//}
	}
	return 0;
}

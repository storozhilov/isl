#define LIBISL__DEBUGGING_ON 1

#include <isl/Log.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/Exception.hxx>
#include <isl/Http.hxx>
#include <isl/DirectLogger.hxx>
#include <isl/StreamLogTarget.hxx>
#include <isl/HttpMessageStreamReader.hxx>
#include <isl/HttpRequestParser.hxx>
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
	isl::DirectLogger logger;						// Logging setup
	isl::StreamLogTarget coutTarget(logger, std::cout);
	isl::Log::debug().connect(coutTarget);
	isl::Log::warning().connect(coutTarget);
	isl::Log::error().connect(coutTarget);
	isl::TcpSocket s;
	s.open();
	s.bind(isl::TcpAddrInfo(isl::TcpAddrInfo::IpV4, isl::TcpAddrInfo::WildcardAddress, LISTEN_PORT));
	s.listen(LISTEN_BACKLOG);
	const isl::Timeout timeout(TRANSMISSION_SECONDS_TIMEOUT);
	while (true) {
		std::auto_ptr<isl::TcpSocket>ss(s.accept(isl::Timeout(ACCEPT_SECONDS_TIMEOUT)));
		if (!ss.get()) {
			std::cout << "Listen timeout has been expired" << std::endl;
			continue;
		}
		isl::HttpRequestParser requestParser;
		isl::HttpMessageStreamReader r(requestParser);
		unsigned int totalBytesRead = 0;
		char buf[BUFFER_SIZE];
		try {
			while (!r.parser().isCompleted()) {
				size_t bytesReadFromDevice;
				std::pair<bool, size_t> res = r.read(*ss.get(), isl::Timestamp::limit(timeout), buf, BUFFER_SIZE, &bytesReadFromDevice);
				if (bytesReadFromDevice <= 0) {
					throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, "Receiving data timeout expired"));
				}
				if (res.second > 0) {
					std::cout.write(buf, res.second);
					std::cout.flush();
					totalBytesRead += res.second;
				}
			}
			std::cout << "Source filename is \"" << requestParser.uri() << '"' << std::endl;
			std::cout << "Current directory is \"" << isl::Http::paramValue(requestParser.header(), "X-Current-Directory") << '"' << std::endl;
			std::cout << "Target filename/directory is \"" << isl::Http::paramValue(requestParser.header(), "X-Dest-Filename") << '"' << std::endl;
			isl::HttpResponseStreamWriter w;
			w.setHeaderField("X-Copy-Status", "OK");
			/*if (!w.writeBodyless()) {
				while (!w.flush());
			}*/
			w.writeBodyless(*ss.get(), isl::Timestamp::limit(isl::Timeout(TRANSMISSION_SECONDS_TIMEOUT)));
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

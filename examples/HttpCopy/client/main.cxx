#define LIBISL__DEBUGGING_ON 1

#include <isl/TcpSocket.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/HttpRequestStreamWriter.hxx>
#include <isl/HttpMessageStreamReader.hxx>
#include <isl/HttpResponseParser.hxx>
#include <isl/String.hxx>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

#define CONNECT_ADDR "127.0.0.1"
#define CONNECT_PORT 8081
#define TRANSMISSION_SECONDS_TIMEOUT 60
//#define BUFFER_SIZE 4096
#define BUFFER_SIZE 256

int main(int argc, char *argv[])
{
	if (argc < 3) {
		std::wcout << "Usage: htcp <source_filename> <dest_filename>" << std::endl;
		return 1;
	}
	//try {
		char cwd[FILENAME_MAX];
		if (!getcwd(cwd, FILENAME_MAX)) {
			throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, "Error fetching current working directory"));
		}
		std::ifstream f(argv[1], std::ios_base::in | std::ios::binary);
		if (f.fail()) {
			throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, "Error opening file to copy"));
		}
		isl::TcpSocket s;
		s.open();
		s.connect(isl::TcpAddrInfo(isl::TcpAddrInfo::IpV4, CONNECT_ADDR, CONNECT_PORT));
		isl::HttpRequestStreamWriter w(argv[1], "PUT");
		w.setHeaderField("X-Current-Directory", cwd);
		w.setHeaderField("X-Dest-Filename", argv[2]);
		char buf[BUFFER_SIZE];
		unsigned int bytesRead;
		while ((bytesRead = f.readsome(buf, BUFFER_SIZE)) > 0) {
			if (!w.writeChunk(s, buf, bytesRead, isl::Timestamp::limit(isl::Timeout(TRANSMISSION_SECONDS_TIMEOUT)))) {
				throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, "Sending data timeout expired"));
			}
		}
		if (!w.finalize(s, isl::Timestamp::limit(isl::Timeout(TRANSMISSION_SECONDS_TIMEOUT)))) {
			throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, "Sending data timeout expired"));
		}
		isl::HttpResponseParser responseParser;
		isl::HttpMessageStreamReader r(responseParser);
		const isl::Timeout timeout(TRANSMISSION_SECONDS_TIMEOUT);
		try {
			unsigned int totalBytesRead = 0;
			while (!r.parser().isCompleted()) {
				size_t bytesReadFromDevice;
				std::pair<bool, size_t> res = r.read(s, isl::Timestamp::limit(timeout), buf, BUFFER_SIZE, &bytesReadFromDevice);
				if (bytesReadFromDevice <= 0) {
					throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, "Receiving data timeout expired"));
				}
				if (res.second > 0) {
					std::cout.write(buf, res.second);
					std::cout.flush();
					totalBytesRead += res.second;
				}
			}
		} catch (isl::Exception& e) {
			std::wcerr << e.what() << std::endl;
			std::wcerr << "Parser state is " << responseParser.state() << ", HTTP-version is '" << isl::String::utf8Decode(responseParser.version()) <<
				"', status code is '" << isl::String::utf8Decode(responseParser.statusCode()) <<
				"', reason phrase is '" << isl::String::utf8Decode(responseParser.reasonPhrase()) << '\'' << std::endl;
			return 1;
		} catch (std::exception& e) {
			std::wcerr << e.what() << std::endl;
			std::wcerr << "Parser state is " << r.parser().state() << std::endl;
			return 1;
		} catch (...) {
			std::wcerr << "Unknown copy error" << std::endl;
			std::wcerr << "Parser state is " << r.parser().state() << std::endl;
			return 1;
		}
	/*} catch (isl::Exception& e) {
		std::wcerr << e.debug() << std::endl;
		return 1;
	} catch (std::exception& e) {
		std::wcerr << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::wcerr << "Unknown copy error" << std::endl;
		return 1;
	}*/
	return 0;
}

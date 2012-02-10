#define LIBISL__DEBUGGING_ON 1

#include <isl/TcpSocket.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/HttpRequestStreamWriter.hxx>
#include <isl/HttpResponseStreamReader.hxx>
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
			throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, L"Error fetching current working directory"));
		}
		std::ifstream f(argv[1], std::ios_base::in | std::ios::binary);
		if (f.fail()) {
			throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, L"Error opening file to copy"));
		}
		isl::TcpSocket s;
		s.open();
		s.connect(CONNECT_ADDR, CONNECT_PORT);
		isl::HttpRequestStreamWriter w(s, argv[1], "PUT");
		w.setHeaderField("X-Current-Directory", cwd);
		w.setHeaderField("X-Dest-Filename", argv[2]);
		char buf[BUFFER_SIZE];
		unsigned int bytesRead;
		while ((bytesRead = f.readsome(buf, BUFFER_SIZE)) > 0) {
			if (!w.writeChunk(buf, bytesRead, isl::Timeout(TRANSMISSION_SECONDS_TIMEOUT))) {
				throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, L"Sending data timeout expired"));
			}
		}
		if (!w.finalize(isl::Timeout(TRANSMISSION_SECONDS_TIMEOUT))) {
			throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, L"Sending data timeout expired"));
		}
		isl::HttpResponseStreamReader r(s);
		try {
			unsigned int totalBytesRead = 0;
			while (!r.isCompleted()) {
				bool timeoutExpired;
				unsigned int bytesRead = r.read(buf, BUFFER_SIZE, isl::Timeout(TRANSMISSION_SECONDS_TIMEOUT), &timeoutExpired);
				if (timeoutExpired) {
					throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, L"Receiving data timeout expired"));
				}
				if (bytesRead > 0) {
					std::cout.write(buf, bytesRead);
					std::cout.flush();
				}
				totalBytesRead += bytesRead;
			}
		} catch (isl::Exception& e) {
			std::wcerr << e.debug() << std::endl;
			std::wcerr << "Parser state is " << r.parserState() << ", HTTP-version is '" << isl::String::utf8Decode(r.version()) <<
				"', status code is '" << isl::String::utf8Decode(r.statusCode()) <<
				"', reason phrase is '" << isl::String::utf8Decode(r.reasonPhrase()) << '\'' << std::endl;
			return 1;
		} catch (std::exception& e) {
			std::wcerr << e.what() << std::endl;
			std::wcerr << "Parser state is " << r.parserState() << std::endl;
			return 1;
		} catch (...) {
			std::wcerr << L"Unknown copy error" << std::endl;
			std::wcerr << "Parser state is " << r.parserState() << std::endl;
			return 1;
		}
	/*} catch (isl::Exception& e) {
		std::wcerr << e.debug() << std::endl;
		return 1;
	} catch (std::exception& e) {
		std::wcerr << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::wcerr << L"Unknown copy error" << std::endl;
		return 1;
	}*/
	return 0;
}

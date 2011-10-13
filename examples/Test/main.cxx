#define LIBISL__DEBUGGING_ON 1

#include <isl/LogMessage.hxx>

#include <isl/TcpSocket.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/HttpError.hxx>
#include <isl/FileLogTarget.hxx>
#include <isl/Core.hxx>
#include <isl/Time.hxx>
#include <isl/ArgumentsFormatter.hxx>
//#include <isl/Timeout.hxx>
//#include <isl/FileLogTarget.hxx>
//#include <isl/Core.hxx>
//#include <isl/AbstractHTTPTask.hxx>
//#include "SourceBrowserServer.hxx"

//#include <isl/Variant.hxx>
//#include <isl/Format.hxx>
//#include <isl/Utf8TextCodec.hxx>
#include <iostream>
#include <stdexcept>
//#include <stdlib.h>

#include <errno.h>

//#include <isl/TcpSocket.hxx>
//#include <isl/HttpRequestStreamReader.hxx>
//#include <iostream>

#define BUFFER_SIZE 4096
#define LISTEN_PORT 8081
#define LISTEN_BACKLOG 10
#define ACCEPT_SECONDS_TIMEOUT 1
#define READ_SECONDS_TIMEOUT 5
//#define BUFFERED_READING false
#define BUFFERED_READING true

int main(int argc, char *argv[])
{
	std::cout << "Test executable has been started" << std::endl;

	/*isl::Core::debugLog.setPrefix(L"DEBUG");
	isl::Core::debugLog.connectTarget(isl::FileLogTarget("test.log"));
	isl::Core::debugLog.log("std::string debug log entry");
	isl::Core::debugLog.log(L"std::wstring debug log entry");
	isl::Core::debugLog.log(isl::LogMessage(L"LogMessage debug log entry"));
	isl::Core::debugLog.log(isl::DebugLogMessage(SOURCE_LOCATION_ARGS, L"DebugLogMessage debug log entry"));
	isl::Exception e1(isl::SystemCallError(SOURCE_LOCATION_ARGS, isl::SystemCallError::PThreadCreate, EAGAIN));
	isl::Core::debugLog.log(isl::ExceptionLogMessage(SOURCE_LOCATION_ARGS, e1, "ExceptionLogMessage debug log entry"));
	isl::Core::debugLog.log(isl::ExceptionLogMessage(SOURCE_LOCATION_ARGS, e1));
	std::runtime_error e2("Foobar");
	isl::Core::debugLog.log(isl::ExceptionLogMessage(SOURCE_LOCATION_ARGS, e2, L"ExceptionLogMessage debug log entry"));
	isl::Core::debugLog.log(isl::ExceptionLogMessage(SOURCE_LOCATION_ARGS, e2));

	isl::SystemCallError e3(SOURCE_LOCATION_ARGS, isl::SystemCallError::PThreadCreate, EAGAIN);
	std::wcout << e3.message() << std::endl;
	std::wcout << e3.debug() << std::endl;
	isl::HttpError e4(SOURCE_LOCATION_ARGS, isl::HttpError::BadRequest, L"Very bad request...");
	std::wcout << e4.message() << std::endl;
	std::wcout << e4.debug() << std::endl;*/

	isl::Time t(isl::Time::now());
	std::wcout << t.toWString(L"%H:%M:%S") << std::endl;
	std::wcout << t.toWString(L"%I:%M:%S") << std::endl;
	std::wcout << t.toWString(L"%l:%M:%S") << std::endl;
	std::wcout << t.toWString(L"%T") << std::endl;
	std::wcout << t.toWString(L"%R") << std::endl;

	//isl::Variant v(1);
	std::wcout << isl::WArgumentsFormatter(L"int value = $0, string value = '$1', double value = $2").arg(isl::Variant(1)).arg(isl::Variant(std::wstring(L"FooBar"))).arg(isl::Variant(24.5)).compose() << std::endl;
	//std::cout << isl::ArgumentsFormatter("int value = $0, string value = '$1'").arg(isl::Variant(1)).arg(isl::Variant(std::string("FooBar"))).compose() << std::endl;
	//std::cout << isl::ArgumentsFormatter("int value = $0, string value = '$1'").arg(isl::Variant(1)).arg(isl::Variant(std::wstring(L"FooBar"))).compose() << std::endl;

	/*isl::DebugLogMessage dlm(SOURCE_LOCATION_ARGS, L"Test message");
	std::wcout << dlm.compose() << std::endl;

	std::wcout << isl::ExceptionLogMessage(SOURCE_LOCATION_ARGS, e1).compose() << std::endl;
	std::runtime_error e2("Foobar");
	std::wcout << isl::ExceptionLogMessage(SOURCE_LOCATION_ARGS, e2).compose() << std::endl;
	std::wcout << isl::ExceptionLogMessage(SOURCE_LOCATION_ARGS, e2, L"Blah-blah-blah").compose() << std::endl;*/
	return 0;


	isl::TcpSocket s;
	s.open();
	s.bind(LISTEN_PORT);
	s.listen(LISTEN_BACKLOG);
	//char buf[BUFFER_SIZE];
	
	while (true) {
		isl::TcpSocket * ss = s.accept(isl::Timeout(ACCEPT_SECONDS_TIMEOUT));
		if (!ss) {
			std::cout << "Listen timeout has been expired" << std::endl;
			continue;
		}
		char buf[BUFFER_SIZE];
		while (true) {
			try {
				ss->ungetChar('r');
				ss->ungetChar('a');
				ss->ungetChar('B');
				ss->ungetChar('o');
				ss->ungetChar('o');
				ss->ungetChar('F');

				if (BUFFERED_READING) {
					unsigned int bytesRead = ss->read(buf, BUFFER_SIZE, isl::Timeout(READ_SECONDS_TIMEOUT));
					if (bytesRead <= 0) {
						std::cout << "Read timeout has been expired" << std::endl;
					} else {
						std::cout << std::string(buf, bytesRead) << std::endl;
					}
				} else {
					char ch;
					if (ss->getChar(ch, isl::Timeout(READ_SECONDS_TIMEOUT))) {
						std::cout << ch;
						if (ch == 10) {
							std::cout << std::flush;
						}
					} else {
						std::cout << "Read timeout has been expired" << std::endl;
					}
				}
			} catch (isl::Exception& e) {
				std::cerr << e.what() << std::endl;
				break;
			}
		}
		delete ss;
	}
	/*std::wcout << isl::WFormat(L"$0, '$1', ${a{b}c}0, $$, ${sdasd{}}_, ${efg}1").arg(isl::Variant(2.5)).arg(isl::Variant(std::wstring(L"foo"))).compose() << std::endl;
	std::wcout << isl::Utf8TextCodec().decode(isl::Format("$0, '$1', ${a{b}c}0, $$, ${sdasd{}}_, ${efg}1").arg(isl::Variant(2.6)).arg(isl::Variant(std::wstring(L"bar"))).compose()) << std::endl;
	std::wcout << isl::Utf8TextCodec().decode(isl::Format("$0, '$1', ${a{b}c}0, $$, ${sdasd{}}_, ${efg}1").arg(isl::Variant(2.7)).arg(isl::Variant(std::string("foobar"))).compose()) << std::endl;
	isl::WFormat();
	exit(0);*/


	//isl::TcpSocket s;
	//isl::HttpRequestStreamReader r(s);
	//isl::HttpRequestStreamReader::ParserState st = r.parserState();
	//std::vector<int> v(100);
	//std::cout << v.size() << std::endl;
	//exit(0);
	std::cout << "Test executable has been terminated" << std::endl;
}

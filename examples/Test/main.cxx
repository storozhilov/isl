#define LIBISL__DEBUGGING_ON 1

// TODO Migrate test to Goole Test library

#include <isl/common.hxx>
#include <isl/LogMessage.hxx>
#include <isl/TcpAddrInfo.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/Http.hxx>
#include <isl/FileLogTarget.hxx>
#include <isl/DateTime.hxx>
//#include <isl/VariantFormatter.hxx>
//#include <isl/HttpRequestStreamReader.hxx>
#include <isl/AbstractMessageConsumer.hxx>
#include <isl/MessageProvider.hxx>
#include <isl/MessageFan.hxx>
#include <isl/MessageBus.hxx>
#include <isl/MessageQueue.hxx>
#include <isl/MessageBuffer.hxx>
#include <isl/AbstractMessageBrokerService.hxx>
#include <isl/AbstractMessageBrokerConnection.hxx>
//#include <isl/Timeout.hxx>
//#include <isl/FileLogTarget.hxx>
//#include <isl/AbstractHTTPTask.hxx>
//#include "SourceBrowserServer.hxx"

//#include <isl/Variant.hxx>
//#include <isl/Format.hxx>
//#include <isl/Utf8TextCodec.hxx>
#include <iostream>
#include <stdexcept>
#include <iomanip>
//#include <stdlib.h>

#include <errno.h>

//#include <isl/TcpSocket.hxx>
//#include <isl/HttpRequestStreamReader.hxx>
//#include <iostream>

#define BUFFER_SIZE 4096
#define LISTEN_PORT 8081
#define LISTEN_BACKLOG 10
#define ACCEPT_SECONDS_TIMEOUT 1
#define READ_SECONDS_TIMEOUT 60
//#define BUFFERED_READING false
#define BUFFERED_READING true

void testTimeout()
{
	//isl::Timeout to1(2, 100000000);
	/*isl::Timeout to1(1, 100000000);
	std::cout << to1.seconds() << '.' << std::setw(9) << std::setfill('0') << to1.nanoSeconds() << std::endl;
	timespec limit = to1.limit();
	sleep(1);
	isl::Timeout to2 = isl::Timeout::leftToLimit(limit);
	std::cout << to2.seconds() << '.' << std::setw(9) << std::setfill('0') << to2.nanoSeconds() << std::endl;*/
}

void testTime()
{
	/*isl::Time t1(0, 0, 1, 1);
	std::cout << "t1 = " << t1.toString() << std::endl;
	isl::Time t2 = t1.addNanoSeconds(-2000000002);
	std::cout << "t2 = " << t2.toString() << std::endl;
	isl::Time t3 = t1.addSeconds(-2);
	std::cout << "t3 = " << t3.toString() << std::endl;
	isl::Time t4 = t1.addNanoSeconds(1000000023);
	std::cout << "t4 = " << t4.toString() << std::endl;
	time_t now = time(0);
	isl::Time t5(now, true, 123);
	std::cout << "t5 = " << t5.toString() << std::endl;
	isl::Time t6(isl::Time::now());
	std::cout << "t6 = " << t6.toString() << std::endl;
	isl::Time t7("10:11:12.123456");
	std::cout << "t7 = " << t7.toString() << std::endl;
	isl::Time t8("10:11:12.1p");
	std::cout << "t8 = " << t8.toString() << std::endl;
	isl::Time t9("10:11:12.p");
	std::cout << "t9 = " << t9.toString() << std::endl;*/
}

void testDateTime()
{
	/*const wchar_t * wfmt = L"%Y-%m-%d %H:%M:%S %z";
	isl::DateTime dt1 = isl::DateTime::now();
	std::wcout << dt1.toWString(wfmt) << std::endl;
	std::wcout << dt1.toWString(isl::DateTime::HttpOutputWFormat) << std::endl;
	isl::Time t1 = isl::Time::now();
	std::wcout << t1.toWString(wfmt) << std::endl;*/

	//const wchar_t * inTimeFormat = L"%H:%M:%S.%f";
	//const wchar_t * outTimeFormat = L"%H/%M/%S.%f";
	//std::wcout << "Time is = " << isl::Time::fromWString(L"12:25:33.432", inTimeFormat).toWString(outTimeFormat) << std::endl;

	//const wchar_t * inDateFormat = L"%Y-%m-%d";
	//const wchar_t * outDateFormat = L"%d.%m.%Y";
	//std::wcout << "Date is " << isl::Date::fromWString(L"1974-12-28", inDateFormat).toWString(outDateFormat) << std::endl;

	//const wchar_t * inDateTimeFormat = L"%Y-%m-%d %H:%M:%S.%f%z";
	////const wchar_t * outDateTimeFormat = L"%d.%m.%Y %H/%M/%S.%f%z";
	//const wchar_t * outDateTimeFormat = isl::Http::DateTimeWFormat;
	//std::wcout << "Date is " << isl::DateTime::fromWString(L"1974-12-28 12:33:21.436+0400", inDateTimeFormat).toWString(outDateTimeFormat) << std::endl;
}

void testVariant()
{
/*	isl::Variant v(1);
	std::wcout << isl::VariantWFormatter(L"int value = $0, string value = '$1', double value = $2, date value = $3, time value = $4, datetime value = $5").arg(isl::Variant(1)).arg(isl::Variant(std::wstring(L"FooBar"))).arg(isl::Variant(24.5)).arg(isl::Variant(isl::Date::now())).arg(isl::Variant(isl::Time::now())).arg(isl::Variant(isl::DateTime::now())).compose() << std::endl;
	std::wcout << isl::VariantWFormatter(L"char value = $0, wchar_t value = $1").arg(isl::Variant('i')).arg(isl::Variant(L's')).compose() << std::endl;
	std::wcout << isl::VariantWFormatter(L"char value = $0, wchar_t value = $1").arg('i').arg(L's').compose() << std::endl;*/
	//std::cout << isl::ArgumentsFormatter("int value = $0, string value = '$1'").arg(isl::Variant(1)).arg(isl::Variant(std::string("FooBar"))).compose() << std::endl;
	//std::cout << isl::ArgumentsFormatter("int value = $0, string value = '$1'").arg(isl::Variant(1)).arg(isl::Variant(std::wstring(L"FooBar"))).compose() << std::endl;
}

void testHttpRequestStreamReader()
{
//	isl::TcpSocket s;
//	s.open();
//	s.bind(LISTEN_PORT);
//	s.listen(LISTEN_BACKLOG);
//	//char buf[BUFFER_SIZE];
//	while (true) {
//		isl::TcpSocket * ss = s.accept(isl::Timeout(ACCEPT_SECONDS_TIMEOUT));
//		if (!ss) {
//			std::cout << "Listen timeout has been expired" << std::endl;
//			continue;
//		}
//		isl::HttpRequestStreamReader r(*ss);
//		char buf[BUFFER_SIZE];
//		std::string body;
//		try {
//			while (!r.requestCompleted()) {
//				if (r.invalidRequest()) {
//					throw std::runtime_error("Invalid request");
//				}
//				unsigned int bytesRead = r.read(buf, BUFFER_SIZE, isl::Timeout(READ_SECONDS_TIMEOUT));
//				if (bytesRead > 0) {
//					body.append(buf, bytesRead);
//				}
//			}
//		} catch (isl::Exception& e) {
//			std::cerr << "Error occured: '" << isl::String::utf8Encode(e.debug()) << '\'' << std::endl;
//		} catch (std::exception& e) {
//			std::cerr << "Error occured: '" << e.what() << '\'' << std::endl;
//		} catch (...) {
//			std::cerr << "Unknown error occured." << std::endl;
//		}
//		std::cout << "HTTP-request has been read. Method: '" << r.method() << "', URI: '" << r.uri() << "' Version: '" << r.version() << "', Headers:" << std::endl;
//		isl::HttpRequestStreamReader::Header header = r.header();
//		for (isl::HttpRequestStreamReader::Header::const_iterator i = header.begin(); i != header.end(); ++i) {
//			std::cout << "\t'" << i->first << "': '" << i->second << '\'' << std::endl;
//		}
//		std::cout << "Body is:\n--\n" << body << "\n--" << std::endl;
//		/*while (true) {
//			try {
//				ss->ungetChar('r');
//				ss->ungetChar('a');
//				SS->ungetChar('B');
//				ss->ungetChar('o');
//				ss->ungetChar('o');
//				ss->ungetChar('F');
//
//				if (BUFFERED_READING) {
//					unsigned int bytesRead = ss->read(buf, BUFFER_SIZE, isl::Timeout(READ_SECONDS_TIMEOUT));
//					if (bytesRead <= 0) {
//						std::cout << "Read timeout has been expired" << std::endl;
//					} else {
//						std::cout << std::string(buf, bytesRead) << std::endl;
//					}
//				} else {
//					char ch;
//					if (ss->getChar(ch, isl::Timeout(READ_SECONDS_TIMEOUT))) {
//						std::cout << ch;
//						if (ch == 10) {
//							std::cout << std::flush;
//						}
//					} else {
//						std::cout << "Read timeout has been expired" << std::endl;
//					}
//				}
//			} catch (isl::Exception& e) {
//				std::cerr << e.what() << std::endl;
//				break;
//			}
//		}*/
//		delete ss;
//	}
}

void dumpAddrInfo(const isl::TcpAddrInfo& ai)
{
	std::cout << ai.host();
	if (!ai.service().empty()) {
		std::cout << ':' << ai.service();
	} else if (ai.port() > 0) {
		std::cout << ':' << ai.port();
	}
	std::cout << ", canonical name: " << (ai.canonicalName().empty() ? "<not defined>" : ai.canonicalName()) << ": ";
	for (isl::TcpAddrInfo::EndpointList::const_iterator i = ai.endpoints().begin(); i != ai.endpoints().end(); ++i) {
		if (i != ai.endpoints().begin()) {
			std::cout << ", ";
		}
		std::cout << i->host << ':' << i->port;
	}
	std::cout << std::endl;
}

void testTcpEndpoint()
{
	isl::TcpAddrInfo ep1(isl::TcpAddrInfo::IpV4, "192.168.1.1", "http");
	dumpAddrInfo(ep1);
	isl::TcpAddrInfo epCopy(ep1);
	dumpAddrInfo(epCopy);
	isl::TcpAddrInfo ep2(isl::TcpAddrInfo::IpV4, isl::TcpAddrInfo::LoopbackAddress, "http");
	dumpAddrInfo(ep2);
	epCopy = ep2;
	dumpAddrInfo(epCopy);
	isl::TcpAddrInfo ep3(isl::TcpAddrInfo::IpV4, isl::TcpAddrInfo::WildcardAddress, "http");
	dumpAddrInfo(ep3);
	isl::TcpAddrInfo ep4(isl::TcpAddrInfo::IpV4, "192.168.1.1", 8080);
	dumpAddrInfo(ep4);
	isl::TcpAddrInfo ep5(isl::TcpAddrInfo::IpV4, isl::TcpAddrInfo::LoopbackAddress, 8080);
	dumpAddrInfo(ep5);
	isl::TcpAddrInfo ep6(isl::TcpAddrInfo::IpV4, isl::TcpAddrInfo::WildcardAddress, 8080);
	dumpAddrInfo(ep6);
}

int main(int argc, char *argv[])
{
	std::cout << "Test executable has been started" << std::endl;

	isl::debugLog().connectTarget(isl::FileLogTarget("test.log"));

	//testDateTime();
	testTimeout();
	testTime();
	//testTcpEndpoint();
	return 0;
	//testVariant();
	//testHttpRequestStreamReader();

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

	/*isl::Time t(isl::Time::now());
	std::wcout << t.toWString(L"%H:%M:%S") << std::endl;
	std::wcout << t.toWString(L"%I:%M:%S") << std::endl;
	std::wcout << t.toWString(L"%l:%M:%S") << std::endl;
	std::wcout << t.toWString(L"%T") << std::endl;
	std::wcout << t.toWString(L"%R") << std::endl;
	std::cout << t.toString("%H:%M:%S") << std::endl;
	std::cout << t.toString("%I:%M:%S") << std::endl;
	std::cout << t.toString("%l:%M:%S") << std::endl;
	std::cout << t.toString("%T") << std::endl;
	std::cout << t.toString("%R") << std::endl;

	const wchar_t * wfmt = L"%b, %B, %a, %A, %C, %d, %D, %e, %F, %g, %G, %h, %j, %m, %u, %U, %V, %w, %W, %x, %y, %Y";
	const char * fmt = "%b, %B, %a, %A, %C, %d, %D, %e, %F, %g, %G, %h, %j, %m, %u, %U, %V, %w, %W, %x, %y, %Y";
	isl::Date d1(isl::Date::now());
	isl::Date d2(1999, 12, 26);
	isl::Date d3(1999, 12, 27);
	isl::Date d4(2000, 1, 1);
	std::wcout << d1.toWString(wfmt) << std::endl;
	std::wcout << d2.toWString(wfmt) << std::endl;
	std::wcout << d3.toWString(wfmt) << std::endl;
	std::wcout << d4.toWString(wfmt) << std::endl;
	std::cout << d1.toString(fmt) << std::endl;
	std::cout << d2.toString(fmt) << std::endl;
	std::cout << d3.toString(fmt) << std::endl;
	std::cout << d4.toString(fmt) << std::endl;
	//std::wcout << d1.secondsFromEpoch() << ", " << time(NULL) << std::endl;
	//isl::Date d3(1999, 12, 26);
	//std::wcout << d3.toWString(wfmt) << std::endl;
	//isl::Date d4(1999, 12, 27);
	//std::wcout << d4.toWString(wfmt) << std::endl;
	//isl::Date d2(2000, 1, 1);
	//std::wcout << d2.toWString(wfmt) << std::endl;
	////isl::Date d5(1970, 1, 3);
	//isl::Date d5(1969, 12, 31);
	//std::wcout << d5.secondsFromEpoch() << std::endl;

	//isl::Variant v(1);
	std::wcout << isl::WArgumentsFormatter(L"int value = $0, string value = '$1', double value = $2").arg(isl::Variant(1)).arg(isl::Variant(std::wstring(L"FooBar"))).arg(isl::Variant(24.5)).compose() << std::endl;
	//std::cout << isl::ArgumentsFormatter("int value = $0, string value = '$1'").arg(isl::Variant(1)).arg(isl::Variant(std::string("FooBar"))).compose() << std::endl;
	//std::cout << isl::ArgumentsFormatter("int value = $0, string value = '$1'").arg(isl::Variant(1)).arg(isl::Variant(std::wstring(L"FooBar"))).compose() << std::endl;*/

	/*isl::DebugLogMessage dlm(SOURCE_LOCATION_ARGS, L"Test message");
	std::wcout << dlm.compose() << std::endl;

	std::wcout << isl::ExceptionLogMessage(SOURCE_LOCATION_ARGS, e1).compose() << std::endl;
	std::runtime_error e2("Foobar");
	std::wcout << isl::ExceptionLogMessage(SOURCE_LOCATION_ARGS, e2).compose() << std::endl;
	std::wcout << isl::ExceptionLogMessage(SOURCE_LOCATION_ARGS, e2, L"Blah-blah-blah").compose() << std::endl;*/
	return 0;


//	isl::TcpSocket s;
//	s.open();
//	s.bind(LISTEN_PORT);
//	s.listen(LISTEN_BACKLOG);
//	//char buf[BUFFER_SIZE];
//	
//	while (true) {
//		std::auto_ptr<isl::TcpSocket> ss(s.accept(isl::Timeout(ACCEPT_SECONDS_TIMEOUT)));
//		if (!ss.get()) {
//			std::cout << "Listen timeout has been expired" << std::endl;
//			continue;
//		}
//		char buf[BUFFER_SIZE];
//		while (true) {
//			try {
//				ss->ungetChar('r');
//				ss->ungetChar('a');
//				ss->ungetChar('B');
//				ss->ungetChar('o');
//				ss->ungetChar('o');
//				ss->ungetChar('F');
//
//				if (BUFFERED_READING) {
//					unsigned int bytesRead = ss->read(buf, BUFFER_SIZE, isl::Timeout(READ_SECONDS_TIMEOUT));
//					if (bytesRead <= 0) {
//						std::cout << "Read timeout has been expired" << std::endl;
//					} else {
//						std::cout << std::string(buf, bytesRead) << std::endl;
//					}
//				} else {
//					char ch;
//					if (ss->getChar(ch, isl::Timeout(READ_SECONDS_TIMEOUT))) {
//						std::cout << ch;
//						if (ch == 10) {
//							std::cout << std::flush;
//						}
//					} else {
//						std::cout << "Read timeout has been expired" << std::endl;
//					}
//				}
//			} catch (isl::Exception& e) {
//				std::cerr << e.what() << std::endl;
//				break;
//			}
//		}
//	}
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

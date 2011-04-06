#define LIBISL__DEBUGGING_ON 1

#include <isl/FileLogTarget.hxx>
#include <isl/Core.hxx>
#include <isl/AbstractHTTPTask.hxx>
#include "SourceBrowserServer.hxx"

#include <isl/Variant.hxx>
#include <isl/Format.hxx>
#include <iostream>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	/*isl::Variant v1(1);
	v1 = 2.5;
	isl::Variant v2(std::wstring(L"Wide string variant"));
	isl::Variant v3(std::string("String variant"));
	std::wcout << L"Value: '" << v1.value<int>() << L"', serialized value: '" << v1.serializedValue() << L"', formatted value: '" << v1.format(L"") << "'" << std::endl;
	std::wcout << L"Value: '" << v2.value<std::wstring>() << L"', serialized value: '" << v2.serializedValue() << L"', formatted value: '" << v2.format(L"") << "'" << std::endl;
	std::wcout << L"Value: '" << v3.value<std::wstring>() << L"', serialized value: '" << v3.serializedValue() << L"', formatted value: '" << v3.format(L"") << "'" << std::endl;
	v2.resetValue();
	std::wcout << L"Value: '" << v2.value<std::wstring>() << L"', serialized value: '" << v2.serializedValue() << L"', formatted value: '" << v2.format(L"") << "'" << std::endl;
	std::wcout << "Test" << std::endl;*/

	///isl::BasicFormat<wchar_t> fmt(L"First agument is $0");
	//isl::BasicFormat<wchar_t>(L"First agument is $0").arg(isl::Variant(2.5)).compose();
	//isl::Format("First agument is $0").arg(isl::Variant(2.5)).compose();

	std::cout << isl::Format("First agument is $0, $$").arg(isl::Variant(2.5)).compose()  << std::endl;
	std::wcout << isl::WFormat(L"First agument is $0, $$").arg(isl::Variant(2.5)).compose()  << std::endl;
	std::wcout << isl::WFormat(L"$0, '$1', $2, $$").arg(isl::Variant(2.5)).arg(isl::Variant(L"foobar")).compose()  << std::endl;

	//isl::WFormat(L"First agument is $0").arg(isl::Variant(2.5)).compose();
	//fmt.arg(isl::Variant(2.5));
	//fmt.arg(v1).compose();
	//fmt.compose();
	exit(0);

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

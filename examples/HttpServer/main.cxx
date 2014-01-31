#include <isl/Server.hxx>
#include <isl/PidFile.hxx>
#include <isl/AbstractSyncTcpService.hxx>
#include <isl/Exception.hxx>
#include <isl/HttpRequestReader.hxx>
#include <isl/HttpResponseStreamWriter.hxx>
#include <isl/DirectLogger.hxx>
#include <isl/StreamLogTarget.hxx>
#include <iostream>
#include <sstream>

#define LISTEN_PORT 8888			// TCP-port to listen to
#define MAX_CLIENTS 10				// Max clients to be served simultaneously
#define TRANSMISSION_SECONDS_TIMEOUT 60		// Data transmission timeout in seconds

class HttpService : public isl::AbstractSyncTcpService
{
public:
	HttpService(Subsystem * owner) :
		AbstractSyncTcpService(owner, MAX_CLIENTS)
	{
		// Adding a listener to the service
		addListener(isl::TcpAddrInfo(isl::TcpAddrInfo::IpV4, isl::TcpAddrInfo::WildcardAddress, LISTEN_PORT));
	}
private:
	// Task class which is returning to client a web-page with properties of the HTTP-request he/she issued
	class HttpTask : public AbstractTask
	{
	public:
		HttpTask(isl::TcpSocket& socket) :
			AbstractTask(socket)
		{}
	private:
		HttpTask();
		// Task execution method definition
		virtual void executeImpl(TaskDispatcherType& taskDispatcher)
		{
			isl::HttpRequestParser parser;
			isl::HttpRequestReader reader(parser);
			bool requestFetched = false;
			try {
				size_t bytesReadFromDevice;
				requestFetched = reader.read(socket(), isl::Timestamp::limit(isl::Timeout(TRANSMISSION_SECONDS_TIMEOUT)), &bytesReadFromDevice);
				if (requestFetched) {
					isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Request has been fetched, bytesReadFromDevice = ") << bytesReadFromDevice);
				} else {
					isl::Log::warning().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Request have NOT been fetched, bytesReadFromDevice = ") << bytesReadFromDevice);
					return;
				}
			} catch (std::exception& e) {
				isl::Log::error().log(isl::ExceptionLogMessage(SOURCE_LOCATION_ARGS, e));
				return;
			} catch (...) {
				isl::Log::error().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Unknown error occured"));
				return;
			}
			// Composing an HTTP-response
			std::ostringstream oss;
			oss << "<html><head><title>HTTP-request has been recieved</title></head><body>";
			if (!requestFetched) {
				if (parser.isBad()) {
					oss << "<p>Bad request: &quot;" << parser.error()->message() << "&quot;</p>";
				} else {
					oss << "<p>Timeout expired</p>";
				}
			} else {
				oss << "<p>URI: &quot;" << parser.uri() << "&quot;</p>" <<
					"<p>path: &quot;" << reader.path() << "&quot;</p>" <<
					"<p>query: &quot;" << reader.query() << "&quot;</p>";
				for (isl::Http::Params::const_iterator i = reader.get().begin(); i != reader.get().end(); ++i) {
					oss << "<p>get[&quot;" << i->first << "&quot;] = &quot;" << i->second << "&quot;</p>";
				}
				for (isl::Http::Params::const_iterator i = parser.headers().begin(); i != parser.headers().end(); ++i) {
					oss << "<p>header[&quot;" << i->first << "&quot;] = &quot;" << i->second << "&quot;</p>";
				}
				for (isl::Http::RequestCookies::const_iterator i = reader.cookies().begin(); i != reader.cookies().end(); ++i) {
					oss << "<p>cookie[&quot;" << i->first << "&quot;] = &quot;" << i->second.value << "&quot;</p>";
				}
			}
			oss << "</body></html>";
			// Sending an HTTP-response to the client
			isl::HttpResponseStreamWriter responseWriter;
			responseWriter.setHeaderField("Content-Type", "text/html; charset=utf-8");
			responseWriter.writeOnce(socket(), oss.str(), isl::Timestamp::limit(isl::Timeout(TRANSMISSION_SECONDS_TIMEOUT)));
		}
	};
	// Task creation factory method definition
	virtual AbstractTask * createTask(isl::TcpSocket& socket)
	{
		return new HttpTask(socket);
	}
};

// Our HTTP-server class
class HttpServer : public isl::Server
{
public:
	HttpServer(int argc, char * argv[]) :
		isl::Server(argc, argv),
		_httpService(this)
	{}
private:
	HttpServer();
	HttpServer(const HttpServer&);

	HttpService _httpService;
};

int main(int argc, char *argv[])
{
	isl::PidFile pidFile("hsd.pid");					// Writing PID of the server to file
	isl::DirectLogger logger;						// Logging setup
	isl::StreamLogTarget coutTarget(logger, std::cout);
	isl::Log::debug().connect(coutTarget);
	isl::Log::warning().connect(coutTarget);
	isl::Log::error().connect(coutTarget);
	HttpServer server(argc, argv);						// Creating server object
	server.run();								// Running server
}

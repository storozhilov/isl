#include <isl/Server.hxx>
#include <isl/PidFile.hxx>
#include <isl/AbstractSyncTcpService.hxx>
#include <isl/Exception.hxx>
#include <isl/HttpRequestReader.hxx>
#include <isl/HttpResponseStreamWriter.hxx>
#include <isl/FileLogTarget.hxx>
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
	// Returning HTTP-request properties task class to execute in task dispatcher
	class HttpTask : public AbstractTask
	{
	public:
		HttpTask(isl::TcpSocket& socket) :
			AbstractTask(socket)
		{}
	private:
		HttpTask();
		// Task execution method definition
		virtual void executeImpl(isl::TaskDispatcher<AbstractTask>& taskDispatcher)
		{
			isl::HttpRequestParser parser;
			isl::HttpRequestReader reader(parser);
			bool requestFetched = false;
			try {
				std::clog << "Starting to read HTTP-request" << std::endl;
				size_t bytesReadFromDevice;
				requestFetched = reader.read(socket(), isl::Timestamp::limit(isl::Timeout(TRANSMISSION_SECONDS_TIMEOUT)), &bytesReadFromDevice);
				std::clog << (requestFetched ? "Request has been fetched" : "Request has NOT been fetched") << ", bytesReadFromDevice = " << bytesReadFromDevice << std::endl;
			} catch (std::exception& e) {
				std::cerr << e.what() << std::endl;
				return;
			} catch (...) {
				std::cerr << "Unknown error occured." << std::endl;
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
				for (isl::Http::Params::const_iterator i = parser.header().begin(); i != parser.header().end(); ++i) {
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
	isl::debugLog().connectTarget(isl::FileLogTarget("hsd.log"));		// Connecting basic logs to one file target
	isl::warningLog().connectTarget(isl::FileLogTarget("hsd.log"));
	isl::errorLog().connectTarget(isl::FileLogTarget("hsd.log"));
	HttpServer server(argc, argv);						// Creating server object
	server.run();								// Running server
	isl::debugLog().disconnectTargets();					// Disconnecting basic logs from the targets
	isl::warningLog().disconnectTargets();
	isl::errorLog().disconnectTargets();
}

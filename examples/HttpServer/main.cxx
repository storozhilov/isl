#include <isl/Server.hxx>
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

// Our HTTP-service subsystem class
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
	class HttpTask : public isl::AbstractSyncTcpService::AbstractTask
	{
	public:
		HttpTask(AbstractSyncTcpService& service, isl::TcpSocket& socket) :
			isl::AbstractSyncTcpService::AbstractTask(service, socket),
			_requestReader(socket)
		{}
	private:
		HttpTask();
		// Task execution method definition
		virtual void execute(TaskDispatcherType::WorkerThread& worker)
		{
			// Reading an HTTP-request and reporting an error if occured
			try {
				std::clog << "Starting to read HTTP-request" << std::endl;
				size_t bytesReadFromDevice;
				bool requestFetched = _requestReader.receive(isl::Timeout(TRANSMISSION_SECONDS_TIMEOUT), &bytesReadFromDevice);
				std::clog << (requestFetched ? "Request has been completed" : "Request has NOT been completed") << ", bytesReadFromDevice = " << bytesReadFromDevice << std::endl;
			} catch (isl::Exception& e) {
				std::cerr << e.what() << std::endl;
				isl::HttpResponseStreamWriter responseWriter(socket(), "500");
				responseWriter.setHeaderField("Content-Type", "text/html; charset=utf-8");
				responseWriter.writeOnce(e.what());
				return;
			} catch (std::exception& e) {
				std::cerr << e.what() << std::endl;
				isl::HttpResponseStreamWriter responseWriter(socket(), "500");
				responseWriter.setHeaderField("Content-Type", "text/html; charset=utf-8");
				responseWriter.writeOnce(e.what());
				return;
			} catch (...) {
				std::cerr << "Unknown error occured." << std::endl;
				isl::HttpResponseStreamWriter responseWriter(socket(), "500");
				responseWriter.setHeaderField("Content-Type", "text/html; charset=utf-8");
				responseWriter.writeOnce("Unknown error occured.");
				return;
			}
			// Composing an HTTP-response
			std::ostringstream oss;
			oss << "<html><head><title>HTTP-request has been recieved</title></head><body>";
			if (_requestReader.streamReader().isBad()) {
				oss << "<p>Bad request: &quot;" << _requestReader.streamReader().error()->message() << "&quot;</p>";
			} else {
				oss << "<p>URI: &quot;" << _requestReader.streamReader().uri() << "&quot;</p>" <<
					"<p>path: &quot;" << isl::String::decodePercent(_requestReader.path()) << "&quot;</p>" <<
					"<p>query: &quot;" << _requestReader.query() << "&quot;</p>";
				for (isl::Http::Params::const_iterator i = _requestReader.get().begin(); i != _requestReader.get().end(); ++i) {
					oss << "<p>get[&quot;" << i->first << "&quot;] = &quot;" << i->second << "&quot;</p>";
				}
				for (isl::Http::Params::const_iterator i = _requestReader.streamReader().header().begin(); i != _requestReader.streamReader().header().end(); ++i) {
					oss << "<p>header[&quot;" << i->first << "&quot;] = &quot;" << i->second << "&quot;</p>";
				}
				for (isl::Http::RequestCookies::const_iterator i = _requestReader.cookies().begin(); i != _requestReader.cookies().end(); ++i) {
					oss << "<p>cookie[&quot;" << i->first << "&quot;] = &quot;" << i->second.value << "&quot;</p>";
				}
			}
			oss << "</body></html>";
			// Sending an HTTP-response to the client
			isl::HttpResponseStreamWriter responseWriter(socket());
			responseWriter.setHeaderField("Content-Type", "text/html; charset=utf-8");
			responseWriter.writeOnce(oss.str());
		}

		isl::HttpRequestReader _requestReader;
	};
	// Task creation factory method definition
	virtual isl::AbstractSyncTcpService::AbstractTask * createTask(ListenerThread& /*listener*/, isl::TcpSocket& socket)
	{
		return new HttpTask(*this, socket);
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
	// Some event handlers re-definition
	void beforeStart()
	{
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Starting server"));
	}
	void afterStart()
	{
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Server has been started"));
	}
	void beforeStop()
	{
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Stopping server"));
	}
	void afterStop()
	{
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Server has been stopped"));
	}

	HttpService _httpService;
};

int main(int argc, char *argv[])
{
	isl::writePid("hsd.pid");						// Writing PID of the server to file
	isl::debugLog().connectTarget(isl::FileLogTarget("hsd.log"));		// Connecting basic logs to one file target
	isl::warningLog().connectTarget(isl::FileLogTarget("hsd.log"));
	isl::errorLog().connectTarget(isl::FileLogTarget("hsd.log"));
	HttpServer server(argc, argv);						// Creating server object
	server.run();								// Running server
	isl::debugLog().disconnectTargets();					// Disconnecting basic logs from the targets
	isl::warningLog().disconnectTargets();
	isl::errorLog().disconnectTargets();
}

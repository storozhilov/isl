#define LIBISL__DEBUGGING_ON 1

#include <isl/common.hxx>
#include <isl/AbstractServer.hxx>
#include <isl/SignalHandler.hxx>
//#include <isl/AbstractTcpService.hxx>
#include <isl/AbstractSyncTcpService.hxx>
#include <isl/AbstractAsyncTcpService.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/Exception.hxx>
#include <isl/HttpRequestReader.hxx>
#include <isl/HttpResponseStreamWriter.hxx>
#include <isl/FileLogTarget.hxx>
#include <isl/LogMessage.hxx>
#include <iostream>
#include <sstream>
#include <memory>

#define LISTEN_PORT 8080
#define MAX_CLIENTS 10
#define LISTEN_BACKLOG 10
#define ACCEPT_SECONDS_TIMEOUT 1
#define TRANSMISSION_SECONDS_TIMEOUT 60

/*void testTimeout()
{
	isl::Timeout t1(1, 123, 456, 789);
	std::cout << "t1.seconds() = " << t1.seconds() << ", t1.milliSeconds() = " << t1.milliSeconds() << ", t1.microSeconds() = " << t1.microSeconds() << ", t1.nanoSeconds() = " << t1.nanoSeconds() << std::endl;
	timespec sp = t1.timeSpec();
	std::cout << "sp.tv_sec = " << sp.tv_sec << ", sp.tv_nsec = " << sp.tv_nsec << std::endl;
	timespec l = t1.limit();
	std::cout << "l.tv_sec = " << l.tv_sec << ", l.tv_nsec = " << l.tv_nsec << std::endl;
}*/

/*class HttpService : public isl::AbstractTcpService
{
public:
	HttpService(AbstractSubsystem * owner, unsigned int port, size_t maxClients) :
		AbstractTcpService(owner, maxClients)
	{
		addListener(port);
		addListener(port + 1);
	}
private:
	class HttpTask : public isl::AbstractTcpService::AbstractTask
	{
	public:
		HttpTask(isl::TcpSocket * socket) :
			isl::AbstractTcpService::AbstractTask(socket),
			_requestReader(*socket)
		{}
	private:
		HttpTask();

		virtual void executeImpl(isl::TaskDispatcher::WorkerThread& worker)
		{
			try {
				_requestReader.receive(isl::Timeout(TRANSMISSION_SECONDS_TIMEOUT));
			} catch (isl::Exception& e) {
				std::cerr << isl::String::utf8Encode(e.debug()) << std::endl;
				isl::HttpResponseStreamWriter responseWriter(socket(), "500");
				responseWriter.setHeaderField("Content-Type", "text/html; charset=utf-8");
				responseWriter.writeOnce(isl::String::utf8Encode(e.debug()));
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
			std::ostringstream oss;
			oss << "<html><head><title>HTTP-request has been recieved</title></head><body>" <<
				"<p>URI: &quot;" << _requestReader.uri() << "&quot;</p>" <<
				"<p>path: &quot;" << isl::String::decodePercent(_requestReader.path()) << "&quot;</p>" <<
				"<p>query: &quot;" << _requestReader.query() << "&quot;</p>";
			for (isl::Http::Params::const_iterator i = _requestReader.get().begin(); i != _requestReader.get().end(); ++i) {
				oss << "<p>get[&quot;" << i->first << "&quot;] = &quot;" << i->second << "&quot;</p>";
			}
			for (isl::Http::Params::const_iterator i = _requestReader.header().begin(); i != _requestReader.header().end(); ++i) {
				oss << "<p>header[&quot;" << i->first << "&quot;] = &quot;" << i->second << "&quot;</p>";
			}
			for (isl::Http::RequestCookies::const_iterator i = _requestReader.cookies().begin(); i != _requestReader.cookies().end(); ++i) {
				oss << "<p>cookie[&quot;" << i->first << "&quot;] = &quot;" << i->second.value << "&quot;</p>";
			}
			oss << "</body></html>";
			isl::HttpResponseStreamWriter responseWriter(socket());
			responseWriter.setHeaderField("Content-Type", "text/html; charset=utf-8");
			responseWriter.writeOnce(oss.str());
		}

		isl::HttpRequestReader _requestReader;
	};

	HttpService();

	virtual isl::AbstractTcpService::AbstractTask * createTask(isl::TcpSocket * socket)
	{
		return new HttpTask(socket);
	}
};*/

class HttpService : public isl::AbstractSyncTcpService
{
public:
	HttpService(AbstractSubsystem * owner, unsigned int port, size_t maxClients) :
		AbstractSyncTcpService(owner, maxClients)
	{
		addListener(isl::TcpAddrInfo(isl::TcpAddrInfo::IpV4, isl::TcpAddrInfo::WildcardAddress, port));
		addListener(isl::TcpAddrInfo(isl::TcpAddrInfo::IpV4, isl::TcpAddrInfo::WildcardAddress, port + 1));
	}
private:
	class HttpTask : public isl::AbstractSyncTcpService::AbstractTask
	{
	public:
		HttpTask(AbstractSyncTcpService& service, isl::TcpSocket * socket) :
			isl::AbstractSyncTcpService::AbstractTask(service, socket),
			_requestReader(*socket)
		{}
	private:
		HttpTask();

		virtual void execute(TaskDispatcherType::WorkerThread& worker)
		{
			try {
				_requestReader.receive(isl::Timeout(TRANSMISSION_SECONDS_TIMEOUT));
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
			std::ostringstream oss;
			oss << "<html><head><title>HTTP-request has been recieved</title></head><body>" <<
				"<p>URI: &quot;" << _requestReader.uri() << "&quot;</p>" <<
				"<p>path: &quot;" << isl::String::decodePercent(_requestReader.path()) << "&quot;</p>" <<
				"<p>query: &quot;" << _requestReader.query() << "&quot;</p>";
			for (isl::Http::Params::const_iterator i = _requestReader.get().begin(); i != _requestReader.get().end(); ++i) {
				oss << "<p>get[&quot;" << i->first << "&quot;] = &quot;" << i->second << "&quot;</p>";
			}
			for (isl::Http::Params::const_iterator i = _requestReader.header().begin(); i != _requestReader.header().end(); ++i) {
				oss << "<p>header[&quot;" << i->first << "&quot;] = &quot;" << i->second << "&quot;</p>";
			}
			for (isl::Http::RequestCookies::const_iterator i = _requestReader.cookies().begin(); i != _requestReader.cookies().end(); ++i) {
				oss << "<p>cookie[&quot;" << i->first << "&quot;] = &quot;" << i->second.value << "&quot;</p>";
			}
			oss << "</body></html>";
			isl::HttpResponseStreamWriter responseWriter(socket());
			responseWriter.setHeaderField("Content-Type", "text/html; charset=utf-8");
			responseWriter.writeOnce(oss.str());
		}

		isl::HttpRequestReader _requestReader;
	};

	HttpService();

	virtual std::auto_ptr<isl::AbstractSyncTcpService::AbstractTask> createTask(isl::TcpSocket * socket, ListenerThread& /*listener*/)
	{
		return std::auto_ptr<isl::AbstractSyncTcpService::AbstractTask>(new HttpTask(*this, socket));
	}
};

class HttpServer : public isl::AbstractServer
{
public:
	HttpServer(int argc, char * argv[]) :
		isl::AbstractServer(argc, argv),
		//_startStopMutex(),
		_signalHandler(this),
		//_taskDispatcher(this, 10)//,
		_httpService(this, LISTEN_PORT, MAX_CLIENTS)
	{}
	
	/*virtual void start()
	{
		isl::MutexLocker locker(_startStopMutex);
		setState(IdlingState, StartingState);
		_signalHandler.start();
		_httpService.start();
		setState(StartingState, RunningState);
	}
	virtual void stop()
	{
		isl::MutexLocker locker(_startStopMutex);
		setState(StoppingState);
		_signalHandler.stop();
		_httpService.stop();
		setState(IdlingState);
	}*/
private:
	HttpServer();
	HttpServer(const HttpServer&);

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

	//isl::Mutex _startStopMutex;
	isl::SignalHandler _signalHandler;
	//isl::TaskDispatcher _taskDispatcher;
	HttpService _httpService;
};

int main(int argc, char *argv[])
{
	//testTimeout();
	//return 0;
	//std::cout << isl::String::encodePercent("Вот такая строка") << std::endl;
	//return 0;

	//isl::Core::daemonize();
	isl::writePid("hsd.pid");
	isl::debugLog().connectTarget(isl::FileLogTarget("hsd.log"));
	isl::warningLog().connectTarget(isl::FileLogTarget("hsd.log"));
	isl::errorLog().connectTarget(isl::FileLogTarget("hsd.log"));
	HttpServer server(argc, argv);
	server.run();
	/*isl::TcpSocket s;
	s.open();
	s.bind(LISTEN_PORT);
	s.listen(LISTEN_BACKLOG);
	while (true) {
		std::auto_ptr<isl::TcpSocket>ss(s.accept(isl::Timeout(ACCEPT_SECONDS_TIMEOUT)));
		if (!ss.get()) {
			std::cout << "Listen timeout has been expired" << std::endl;
			continue;
		}
		isl::HttpRequestReader request(*ss.get());
		try {
			//isl::HttpRequestReader 
			request.receive();
		} catch (isl::Exception& e) {
			std::cerr << isl::String::utf8Encode(e.debug()) << std::endl;
			isl::HttpResponseStreamWriter responseWriter(*ss.get(), "500");
			responseWriter.setHeaderField("Content-Type", "text/html; charset=utf-8");
			responseWriter.writeOnce(isl::String::utf8Encode(e.debug()));
			continue;
		} catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
			isl::HttpResponseStreamWriter responseWriter(*ss.get(), "500");
			responseWriter.setHeaderField("Content-Type", "text/html; charset=utf-8");
			responseWriter.writeOnce(e.what());
			continue;
		} catch (...) {
			std::cerr << "Unknown error occured." << std::endl;
			isl::HttpResponseStreamWriter responseWriter(*ss.get(), "500");
			responseWriter.setHeaderField("Content-Type", "text/html; charset=utf-8");
			responseWriter.writeOnce("Unknown error occured.");
			continue;
		}
		std::cout << "HTTP-request has been recieved:" << std::endl <<
			"\tURI: \"" << request.uri() << '"' << std::endl <<
			"\tpath: \"" << isl::String::decodePercent(request.path()) << '"' << std::endl <<
			"\tquery: \"" << request.query() << '"' << std::endl;
		isl::Http::Params get = request.get();
		for (isl::Http::Params::const_iterator i = request.get().begin(); i != request.get().end(); ++i) {
			std::cout << "\tget[\"" << i->first << "\"] = \"" << i->second << '"' << std::endl;
		}
		for (isl::Http::Params::const_iterator i = request.header().begin(); i != request.header().end(); ++i) {
			std::cout << "\theader[\"" << i->first << "\"] = \"" << i->second << '"' << std::endl;
		}
		for (isl::Http::RequestCookies::const_iterator i = request.cookies().begin(); i != request.cookies().end(); ++i) {
			std::cout << "\tcookie[\"" << i->first << "\"] = \"" << i->second.value << '"' << std::endl;
		}
		std::ostringstream oss;
		oss << "<html><head><title>HTTP-request has been recieved</title></head><body>" <<
			"<p>URI: &quot;" << request.uri() << "&quot;</p>" <<
			"<p>path: &quot;" << isl::String::decodePercent(request.path()) << "&quot;</p>" <<
			"<p>query: &quot;" << request.query() << "&quot;</p>";
		for (isl::Http::Params::const_iterator i = request.get().begin(); i != request.get().end(); ++i) {
			oss << "<p>get[&quot;" << i->first << "&quot;] = &quot;" << i->second << "&quot;</p>";
		}
		for (isl::Http::Params::const_iterator i = request.header().begin(); i != request.header().end(); ++i) {
			oss << "<p>header[&quot;" << i->first << "&quot;] = &quot;" << i->second << "&quot;</p>";
		}
		for (isl::Http::RequestCookies::const_iterator i = request.cookies().begin(); i != request.cookies().end(); ++i) {
			oss << "<p>cookie[&quot;" << i->first << "&quot;] = &quot;" << i->second.value << "&quot;</p>";
		}
		oss << "</body></html>";
		isl::HttpResponseStreamWriter responseWriter(*ss.get());
		responseWriter.setHeaderField("Content-Type", "text/html; charset=utf-8");
		responseWriter.writeOnce(oss.str());
	}*/
	isl::debugLog().disconnectTargets();
	isl::warningLog().disconnectTargets();
	isl::errorLog().disconnectTargets();
}

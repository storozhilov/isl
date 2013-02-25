#ifndef ISL__DOXYGEN__HXX
#define ISL__DOXYGEN__HXX 1

//------------------------------------------------------------------------------
// ISL doxygen data header file. Not for any usage.
//
// Copyright (c) 2011-2012, Ilya V. Storozhilov. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//------------------------------------------------------------------------------

namespace isl
{

/*! \mainpage ISL - Internet Server Library, C++ server development toolkit

  \section intro_section Introduction

  Every server application should run daemonized 24x7 in memory leak free multithread environment, have it's own UNIX signals
  handler, thread-safe logging subsystem, thread-safe message queues/buses for inter-thread exchange, etc. In general terms
  the architecture of any server application is usually composed from common design elements which are based on
  particular <a href="http://en.wikipedia.org/wiki/Software_design_pattern">design patterns</a>. Server application developers
  facing the similar challenges where design solutions along with versatile tools must be provided by easy to use toolkit. This
  toolkit is aimed to be an ISL project.

  \section features_section Features

  - Thread class, C++ wrappers for basic inter-thread synchronization objects (mutex, R/W-lock, conditional variable, etc.) and helper classes;
  - Thread-safe multi-target extensible logging architecture implementation with any kind of targets (stdout, file, syslog, database, etc.)
    and multiple target per log support;
  - <a href="http://en.wikipedia.org/wiki/Active_object">Active object pattern</a> templated extensible implementation;
  - Hierarchially organized extensible server/subsystem design in accordance with
    <a href="http://en.wikipedia.org/wiki/Composite_pattern">Composite design pattern</a>;
  - I/O-device abstraction and it's implementation for TCP/UDP(TODO) sockets with asynchronous data transmission and SSL (TODO) support;
  - Extensible UNIX-signal handler subsystem implementation;
  - Extensible design for synchronous (one thread per client connection) and asynchronous (two threads per client connection) TCP-service
    subsystems implementation;
  - Message queueing design elements, including thread-safe message queue/bus/fan, asynchronous
    message broker connection subsystem, asynchronous message broker service subsystem, message routing facilities, etc.;
  - Functionally rich HTTP-module with HTTP-message/HTTP-cookie parsers and composers, HTTP-request/HTTP-response stream readers and writers,
    utility methods, etc.;
  - High-precision extensible timer subsystem to execute tasks periodically;
  - Nanosecond-precision datetime and interval support;
  - SCADA module for device management applications development;
  - Tools for common server application tasks: pidfile saving, daemonizing.

  \section installation_section Installation

  Runtime requirements:

  - libc;
  - libpthread;
  - STL.

  Build requirements:

  - <a href="http://scons.org/doc/production/HTML/scons-user/c95.html">SCons</a>.

  To get ISL's source code do SVN-checkout from the http://svn.storozhilov.com/isl repository:

  \verbatim
  $ svn co http://svn.storozhilov.com/isl
  \endverbatim
  
  To build and install ISL type:

  \verbatim
  $ cd isl/trunk
  $ scons & sudo scons install
  \endverbatim

  To uninstall type:

  \verbatim
  $ scons uninstall
  \endverbatim

  To list build options type:

  \verbatim
  $ scons -h
  \endverbatim

  \section usage_section Usage

  ISL supposes server (see Server) application as a set of subsystems (see Subsystem), which are in turn holding
  the sets of another subsystems and so on. Server itself is a special kind of the subsystem too. Each subsystem
  also operates a set of it's threads (see Subsystem::AbstractThread). Basic subsystem operations are starting
  and stopping ones. All subsystem's threads and subsystems are starting during the start operation and the same
  is for stop operation too. To implement you server you should compose it from your own subsystems/threads and
  already provided ones by ISL.
  
  \section example_section Example
  
  Suppose we want to develop another Apache and should create a starting point for this project. For example, let's
  display to user the details about the HTTP-request he/she issued. So the code snippet will look something like this:

  \code
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
				size_t bytesReadFromDevice;
				requestFetched = reader.read(socket(), isl::Timestamp::limit(isl::Timeout(TRANSMISSION_SECONDS_TIMEOUT)), &bytesReadFromDevice);
				if (requestFetched) {
					isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Request has been fetched, bytesReadFromDevice = ") << bytesReadFromDevice);
				} else {
					isl::Log::warning().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Request has NOT been fetched, bytesReadFromDevice = ") << bytesReadFromDevice);
				}
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
	isl::DirectLogger logger;						// Logging setup
	isl::StreamLogTarget coutTarget(logger, std::cout);
	isl::Log::debug().connect(coutTarget);
	isl::Log::warning().connect(coutTarget);
	isl::Log::error().connect(coutTarget);
	HttpServer server(argc, argv);						// Creating server object
	server.run();								// Running server
}
  \endcode

  \section license_section License

  This software is distributed under &quot;Simplified <a href="http://en.wikipedia.org/wiki/BSD_licenses">BSD-license</a>&quot;
  (A.K.A. &quot;FreeBSD License&quot;) terms. It means you can use it in any application/library you want including commercial one
  with minimum restrictions.
 
  Copyright (c) 2011-2013, <a href="http://storozhilov.com/">Ilya V. Storozhilov</a>. All rights reserved.

  \verbatim
  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
  - Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  \endverbatim

*/

} // namespace smxx

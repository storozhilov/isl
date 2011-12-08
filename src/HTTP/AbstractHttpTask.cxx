#include <isl/AbstractHttpTask.hxx>
#include <isl/Core.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/HttpError.hxx>
//#include <isl/Utf8TextCodec.hxx>

namespace isl
{

//Log AbstractHttpTask::errorLog;
//Log AbstractHttpTask::warningLog;
//Log AbstractHttpTask::debugLog;
//Log AbstractHttpTask::accessLog;

//bool AbstractHttpTask::DefaultKeepAlive = true;
//unsigned int AbstractHttpTask::DefaultMaxKeepAliveRequests = 100;

AbstractHttpTask::AbstractHttpTask(TcpSocket * socket) :
	AbstractTcpService::AbstractTask(socket)//,
	//_request(this),
	//_response(this),
	//_keepAlive(DefaultKeepAlive),								// TODO Use configuration subsystem
	//_maxKeepAliveRequests(DefaultMaxKeepAliveRequests),					// TODO Use configuration subsystem
	//_requestsReceived(0)
{}

//AbstractHttpTask::~AbstractHttpTask()
//{}

bool AbstractHttpTask::methodImplemented(const std::string& method) const
{
	return method == "GET" || method == "POST";
}

bool AbstractHttpTask::versionImplemented(const std::string& version) const
{
	return version == "HTTP/1.0" || version == "HTTP/1.1";
}

void AbstractHttpTask::executeImplementation(Worker& worker)
{
	// TODO
}

void AbstractHttpTask::setMethod(const std::string& method)
{
	if (!methodImplemented(method)) {
		throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::MethodNotImplemented));
	}
	//_request.setMethod(method);								// TODO
}

void AbstractHttpTask::setUri(const std::string& uri)
{
	//_request.setUri(uri);
}

void AbstractHttpTask::setVersion(const std::string& version)
{
	if (!versionImplemented(version)) {
		throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::VersionNotImplemented));
	}
	//_request.setVersion(method);								// TODO
}

//bool AbstractHttpTask::connectionToBeClosed() const
//{
//	// TODO
//	return true;
//}
//
//void AbstractHttpTask::executeImplementation(Worker& worker)
//{
//	std::wostringstream msg;
//	msg << L"Connection established " << socket().localAddress() << L':' <<
//			socket().localPort() << L" (local) <-> " << socket().remoteAddress() << L':' <<
//			socket().remotePort() << L" (remote) with socket descriptor " << socket().descriptor();
//	Core::debugLog.logDebug(SOURCE_LOCATION_ARGS, msg.str());
//	_requestsReceived = 0;
//	while (true) {
//		_request.receive();
//		++_requestsReceived;
//		std::wostringstream sstr;
//		sstr << L"Request for " << Utf8TextCodec().decode(_request.uri()) << L" received from " << socket().remoteAddress();
//		AbstractHttpTask::accessLog.logMessage(sstr.str());
//		_response.generateAndSend();
//		if (connectionToBeClosed()) {
//			break;
//		}
//	}
//}
//
//HTTPResponse::AbstractGenerator * AbstractHttpTask::createGeneratorBadRequest()
//{
//	return new HTTPResponse::GeneratorBadRequest(this);
//}
//
//HTTPResponse::AbstractGenerator * AbstractHttpTask::createGeneratorInternalServerError()
//{
//	return new HTTPResponse::GeneratorInternalServerError(this);
//}

} // namespace isl

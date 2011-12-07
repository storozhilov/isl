#include <isl/AbstractHTTPTask.hxx>
#include <isl/Core.hxx>

namespace isl
{

Log AbstractHTTPTask::errorLog;
Log AbstractHTTPTask::warningLog;
Log AbstractHTTPTask::debugLog;
Log AbstractHTTPTask::accessLog;
bool AbstractHTTPTask::DefaultKeepAlive = true;
unsigned int AbstractHTTPTask::DefaultMaxKeepAliveRequests = 100;

AbstractHTTPTask::AbstractHTTPTask(TcpSocket * socket) :
	AbstractTcpTask(socket),
	_request(this),
	_response(this),
	_keepAlive(DefaultKeepAlive),								// TODO Use configuration subsystem
	_maxKeepAliveRequests(DefaultMaxKeepAliveRequests),					// TODO Use configuration subsystem
	_requestsReceived(0)
{}

//AbstractHTTPTask::~AbstractHTTPTask()
//{}

bool AbstractHTTPTask::connectionToBeClosed() const
{
	// TODO
	return true;
}

void AbstractHTTPTask::executeImplementation(Worker& worker)
{
	std::wostringstream msg;
	msg << L"Connection established " << socket().localAddress() << L':' <<
			socket().localPort() << L" (local) <-> " << socket().remoteAddress() << L':' <<
			socket().remotePort() << L" (remote) with socket descriptor " << socket().descriptor();
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
	_requestsReceived = 0;
	while (true) {
		_request.receive();
		++_requestsReceived;
		std::wostringstream sstr;
		sstr << L"Request for " << String::utf8Decode(_request.uri()) << L" received from " << socket().remoteAddress();
		AbstractHTTPTask::accessLog.log(sstr.str());
		_response.generateAndSend();
		if (connectionToBeClosed()) {
			break;
		}
	}
}

HTTPResponse::AbstractGenerator * AbstractHTTPTask::createGeneratorBadRequest()
{
	return new HTTPResponse::GeneratorBadRequest(this);
}

HTTPResponse::AbstractGenerator * AbstractHTTPTask::createGeneratorInternalServerError()
{
	return new HTTPResponse::GeneratorInternalServerError(this);
}

} // namespace isl


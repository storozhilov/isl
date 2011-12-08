#include <isl/HTTPRequest.hxx>
#include <isl/AbstractHTTPTask.hxx>
#include <isl/Exception.hxx>
#include <isl/TcpSocket.hxx>
#include <isl/IOError.hxx>
#include <stdexcept>

namespace isl
{

HTTPRequest::HTTPRequest(AbstractHTTPTask * task) :
	HTTPMessage(task),
	_keepAliveTimeout(DefaultKeepAliveTimeout),						// TODO Use configuration subsystem
	_maxURISize(DefaultMaxURISize),								// TODO Use configuration subsystem
	_maxHeaderSize(DefaultMaxHeaderSize),							// TODO Use configuration subsystem
	_maxSize(DefaultMaxSize),								// TODO Use configuration subsystem
	_parser(this),
	_method(),
	_uri(),
	_resource(),
	_query(),
	_host(),
	_port(0),
	_get(),
	_cookies(),
	_post(),
	_transferBuffer(DefaultBufferSize, 0)							// TODO Use configuration subsystem
{}
	
HTTPRequest::~HTTPRequest()
{}

AbstractParser::Token HTTPRequest::method() const
{
	return _method;
}

std::string HTTPRequest::uri() const
{
	return _uri;
}

std::string HTTPRequest::resource() const
{
	return _resource;
}

std::string HTTPRequest::query() const
{
	return _query;
}

std::string HTTPRequest::host() const
{
	return _host;
}

int HTTPRequest::port() const
{
	return _port;
}

const std::map<std::string, std::string>& HTTPRequest::get()
{
	return _get;
}

const std::map<std::string, std::string>& HTTPRequest::cookies()
{
	return _cookies;
}

const std::map<std::string, std::string>& HTTPRequest::post()
{
	return _post;
}

bool HTTPRequest::hasCookie(const std::string& cookieName) const
{
	std::map<std::string, std::string>::const_iterator pos = _cookies.find(cookieName);
	return (pos != _cookies.end());
}

std::string HTTPRequest::cookieValue(const std::string& cookieName) const
{
	std::map<std::string, std::string>::const_iterator pos = _cookies.find(cookieName);
	if (pos == _cookies.end()) {
		return std::string();
	}
	return (*pos).second;
}

void HTTPRequest::receive(bool nextKeepAliveCycle)
{
	reset();
	bool firstReading = true;
	// Reading HTTP-request
	while (_parser.needMoreData()) {
		unsigned int bytesRead = _task->socket().read(&_transferBuffer[0], _transferBuffer.size(),
				(nextKeepAliveCycle && firstReading) ? _keepAliveTimeout : _transferTimeout);
		if (bytesRead == 0) {
			return;
		}
		firstReading = false;
		_parser.parse(&_transferBuffer[0], bytesRead);
	}
	// Ignoring the rest of input (TODO: restrict amount of it)
	while (_task->socket().read(&_transferBuffer[0], _transferBuffer.size()) > 0) {}
}

bool HTTPRequest::isBad() const
{
	return _parser.isBadRequest();
}

bool HTTPRequest::isComplete() const
{
	return _parser.isCompleteRequest();
}

void HTTPRequest::reset()
{
	HTTPMessage::reset();
	_parser.reset();
	_method.reset();
	_uri.clear();
	_resource.clear();
	_query.clear();
	_host.clear();
	_port = 0;
	_get.clear();
	_cookies.clear();
	_post.clear();
}

} // namespace isl


#include <isl/HttpRequestReader.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>

namespace isl
{

HttpRequestReader::HttpRequestReader(AbstractIODevice& device, size_t maxBodySize, size_t bufferSize, size_t streamReaderBufferSize) :
	_streamReader(device, streamReaderBufferSize),
	_maxBodySize(maxBodySize),
	_buffer(bufferSize),
	_path(),
	_query(),
	_body(),
	_get(),
	_getExtracted(false),
	_post(),
	_postExtracted(false),
	_cookies(),
	_cookiesExtracted(false)
{}

const Http::RequestCookies& HttpRequestReader::cookies() const
{
	if (!_cookiesExtracted) {
		for (Http::Params::const_iterator i = _streamReader.header().begin(); i != _streamReader.header().end(); ++i) {
			if (i->first != "Cookie") {
				continue;
			}
			HttpRequestCookieParser cookieParser;
			Http::RequestCookies cookies = cookieParser.parse(i->second);
			_cookies.insert(cookies.begin(), cookies.end());
		}
		_cookiesExtracted = true;
	}
	return _cookies;
}

const Http::Params& HttpRequestReader::get() const
{
	if (!_getExtracted) {
		Http::parseParams(_query, _get);
		_getExtracted = true;
	}
	return _get;
}

const Http::Params& HttpRequestReader::post() const
{
	if (!_postExtracted) {
		if (Http::hasParam(_streamReader.header(), "Content-Type", "application/x-www-form-urlencoded")) {
			Http::parseParams(_body, _post);
		}
		_postExtracted = true;
	}
	return _post;
}

void HttpRequestReader::reset()
{
	_streamReader.reset();
	_path.clear();
	_query.clear();
	_body.clear();
	_get.clear();
	_getExtracted = false;
	_post.clear();
	_postExtracted = false;
	_cookies.clear();
	_cookiesExtracted = false;
}

bool HttpRequestReader::receive(Timeout timeout, size_t * bytesReadFromDevice)
{
	if (_streamReader.isCompleted()) {
		reset();
	}
	size_t bodyBytesRead = _streamReader.read(&_buffer[0], _buffer.size(), timeout, bytesReadFromDevice);
	if (_streamReader.isBad()) {
		return false;
	}
	if ((_body.size() + bodyBytesRead) > _maxBodySize) {
		_streamReader.setIsBad(Error(SOURCE_LOCATION_ARGS, "Request entity is too long"));
		return false;
	}
	_body.append(&_buffer[0], bodyBytesRead);
	if (_streamReader.isCompleted()) {
		Http::parseUri(_streamReader.uri(), _path, _query);
		return true;
	} else {
		return false;
	}
}

} // namespace isl

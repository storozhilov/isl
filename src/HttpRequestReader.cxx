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
		Http::grabCookies(_streamReader.parser().header(), _cookies);
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
		if (Http::hasParam(_streamReader.parser().header(), "Content-Type", "application/x-www-form-urlencoded")) {
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
	if (_streamReader.parser().isCompleted()) {
		reset();
	}
	size_t bodyBytesRead = _streamReader.read(&_buffer[0], _buffer.size(), timeout, bytesReadFromDevice);
	if (_streamReader.parser().isBad()) {
		return false;
	}
	if ((_body.size() + bodyBytesRead) > _maxBodySize) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Request entity is too long"));	// TODO ???
	}
	_body.append(&_buffer[0], bodyBytesRead);
	if (_streamReader.parser().isCompleted()) {
		Http::parseUri(_streamReader.uri(), _path, _query);
		return true;
	} else {
		return false;
	}
}

} // namespace isl

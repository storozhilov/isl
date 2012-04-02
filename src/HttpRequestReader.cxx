#include <isl/HttpRequestReader.hxx>
#include <isl/Error.hxx>

namespace isl
{

HttpRequestReader::HttpRequestReader(AbstractIODevice& device) :
	_streamReader(device),
	_buf(),
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

void HttpRequestReader::receive(Timeout timeout, size_t maxBodySize)
{
	reset();
	while (!_streamReader.isCompleted()) {
		bool timeoutExpired;
		size_t bytesRead = _streamReader.read(_buf, BufferSize, timeout, &timeoutExpired);
		if (timeoutExpired) {
			// TODO Use special error class
			throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, L"Timeout expired"));
		}
		if (_streamReader.isBad()) {
			// TODO Use special error class
			throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, _streamReader.parsingError()));
		}
		if ((_body.size() + bytesRead) > maxBodySize) {
			// TODO Use special error class
			throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, L"Request entity is too long"));
		}
		_body.append(_buf, bytesRead);
	}
	Http::parseUri(_streamReader.uri(), _path, _query);
}

} // namespace isl

#include <isl/HttpRequestReader.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>

namespace isl
{

HttpRequestReader::HttpRequestReader(HttpRequestParser& parser, size_t maxBodySize, size_t bufferSize) :
	HttpMessageReader(parser, maxBodySize, bufferSize),
	_parser(parser),
	_path(),
	_query(),
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
		Http::grabCookies(_parser.header(), _cookies);
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
		if (Http::hasParam(_parser.header(), "Content-Type", "application/x-www-form-urlencoded")) {
			Http::parseParams(body(), _post);
		}
		_postExtracted = true;
	}
	return _post;
}

void HttpRequestReader::reset()
{
	HttpMessageReader::reset();
	_path.clear();
	_query.clear();
	_get.clear();
	_getExtracted = false;
	_post.clear();
	_postExtracted = false;
	_cookies.clear();
	_cookiesExtracted = false;
}

void HttpRequestReader::onNewMessage()
{
	_path.clear();
	_query.clear();
	_get.clear();
	_getExtracted = false;
	_post.clear();
	_postExtracted = false;
	_cookies.clear();
	_cookiesExtracted = false;
}

void HttpRequestReader::onCompleteMessage()
{
	Http::parseUri(_parser.uri(), _path, _query);
}

} // namespace isl

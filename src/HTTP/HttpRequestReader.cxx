#include <isl/HttpRequestReader.hxx>
#include <isl/Error.hxx>

namespace isl
{

HttpRequestReader::HttpRequestReader(AbstractIODevice& device) :
	_streamReader(device),
	_path(),
	_query(),
	_body(),
	_get(),
	_post()
{}

void HttpRequestReader::reset()
{
	_streamReader.reset();
	_path.clear();
	_query.clear();
	_body.clear();
	_get.clear();
	_post.clear();
}

void HttpRequestReader::receive(Timeout timeout, unsigned int maxBodySize)
{
	reset();
	char buf[BufferSize];
	while (!_streamReader.isCompleted()) {
		bool timeoutExpired;
		unsigned int bytesRead = _streamReader.read(buf, BufferSize, timeout, &timeoutExpired);
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
		_body.append(buf, bytesRead);
	}
	Http::parseUri(_streamReader.uri(), _path, _query);
	if (!_query.empty()) {
		Http::parseParams(_query, _get);
	}
	if (_streamReader.headerContains("Content-Type", "application/x-www-form-urlencoded")) {
		Http::parseParams(_body, _post);
	}
}

} // namespace isl

#include <isl/HttpRequestStreamReader.hxx>

namespace isl
{

HttpRequestStreamReader::HttpRequestStreamReader(AbstractIODevice& device, size_t bufferSize,
		size_t maxMethodLength, size_t maxUriLength, size_t maxVersionLength) :
	AbstractHttpMessageStreamReader(device, bufferSize),
	_maxMethodLength(maxMethodLength),
	_maxUriLength(maxUriLength),
	_maxVersionLength(maxVersionLength)
{}

HttpMessageParser * HttpRequestStreamReader::createParser() const
{
	return new HttpMessageParser(_maxMethodLength, _maxUriLength, _maxVersionLength);
}

} // namespace isl

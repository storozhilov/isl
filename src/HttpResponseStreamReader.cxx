#include <isl/HttpResponseStreamReader.hxx>

namespace isl
{

HttpResponseStreamReader::HttpResponseStreamReader(AbstractIODevice& device, size_t bufferSize,
		size_t maxVersionLength, size_t maxStatusCodeLength, size_t maxReasonPhraseLength) :
	AbstractHttpMessageStreamReader(device, bufferSize),
	_maxVersionLength(maxVersionLength),
	_maxStatusCodeLength(maxStatusCodeLength),
	_maxReasonPhraseLength(maxReasonPhraseLength)
{}

HttpMessageParser * HttpResponseStreamReader::createParser() const
{
	return new HttpMessageParser(_maxVersionLength, _maxStatusCodeLength, _maxReasonPhraseLength);
}

} // namespace isl

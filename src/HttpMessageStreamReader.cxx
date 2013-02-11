#include <isl/HttpMessageStreamReader.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>

namespace isl
{

HttpMessageStreamReader::HttpMessageStreamReader(HttpMessageParser& parser, size_t bufferSize) :
	_parser(parser),
	_bufferSize(bufferSize),
	_readBuffer(bufferSize),
	_bytesRead(0),
	_bytesParsed(0)
{}

HttpMessageStreamReader::~HttpMessageStreamReader()
{}

void HttpMessageStreamReader::reset()
{
	_parser.reset();
	_bytesRead = 0;
	_bytesParsed = 0;
}

std::pair<bool, size_t> HttpMessageStreamReader::read(AbstractIODevice& device, const Timestamp& limit, char * bodyBuffer, size_t bodyBufferSize, size_t * bytesReadFromDevice)
{
	if (bytesReadFromDevice) {
		*bytesReadFromDevice = 0;
	}
	size_t bodyBytes = 0;
	while (true) {
		if (_bytesParsed < _bytesRead) {
			std::pair<size_t, size_t> res = _parser.parse(&_readBuffer[0] + _bytesParsed, _bytesRead - _bytesParsed, bodyBuffer + bodyBytes, bodyBufferSize - bodyBytes);
			_bytesParsed += res.first;
			bodyBytes += res.second;
			if (_parser.isCompleted()) {
				return std::pair<bool, size_t>(true, bodyBytes);
			}
			if (_parser.isBad() || (bodyBytes >= bodyBufferSize)) {
				// Bad request received or body buffer has no more space
				return std::pair<bool, size_t>(false, bodyBytes);
			}
		} else {
			Timestamp now = Timestamp::now();
			if (now >= limit) {
				// Time limit has been reached
				return std::pair<bool, size_t>(false, bodyBytes);
			}
			_bytesRead = device.read(&_readBuffer[0], _bufferSize, limit - now);
			if (_bytesRead <= 0) {
				// Data read timeout has been expired
				return std::pair<bool, size_t>(false, bodyBytes);
			}
			_bytesParsed = 0;
			if (bytesReadFromDevice) {
				(*bytesReadFromDevice) += _bytesRead;
			}
		}
	}
}

} // namespace isl

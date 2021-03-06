#include <isl/HttpMessageReader.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>

namespace isl
{

HttpMessageReader::HttpMessageReader(HttpMessageParser& parser, size_t maxBodySize, size_t bufferSize) :
	_parser(parser),
	_maxBodySize(maxBodySize),
	_bufferSize(bufferSize),
	_readBuffer(bufferSize),
	_bytesRead(0),
	_bytesParsed(0),
	_bodyBuffer(bufferSize),
	_body()
{}

HttpMessageReader::~HttpMessageReader()
{}

bool HttpMessageReader::read(AbstractIODevice& device, const Timestamp& limit, size_t * bytesReadFromDevice)
{
	if (bytesReadFromDevice) {
		*bytesReadFromDevice = 0;
	}
	while (true) {
		if (_bytesParsed < _bytesRead) {
			if (_parser.isCompleted()) {
				_body.clear();
				onNewMessage();
			}
			// Parsing the rest of the unparsed data in the read buffer
			std::pair<size_t, size_t> res = _parser.parse(&_readBuffer[0] + _bytesParsed, _bytesRead - _bytesParsed, &_bodyBuffer[0], _bufferSize);
			_bytesParsed += res.first;
			if ((_body.size() + res.second) > _maxBodySize) {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "Request entity is too long"));	// Maybe an own error class?
			}
			_body.append(&_bodyBuffer[0], res.second);
			if (_parser.isCompleted()) {
				onCompleteMessage();
				return true;
			}
			if (_parser.isBad() || (Timestamp::now() >= limit)) {
				// Bad request received or the time limit has been reached
				return false;
			}
		} else {
			// Reading next portion of data from the device into the read buffer
			_bytesRead = device.read(&_readBuffer[0], _bufferSize, limit.leftTo());
			if (_bytesRead <= 0) {
				// Data read timeout has been expired
				return false;
			}
			_bytesParsed = 0;
			if (bytesReadFromDevice) {
				(*bytesReadFromDevice) += _bytesRead;
			}
		}
	}
}

void HttpMessageReader::reset()
{
	_parser.reset();
	_bytesRead = 0;
	_bytesParsed = 0;
	_body.clear();
}

} // namespace isl

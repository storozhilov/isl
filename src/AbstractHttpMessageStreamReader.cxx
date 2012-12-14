#include <isl/AbstractHttpMessageStreamReader.hxx>

namespace isl
{

AbstractHttpMessageStreamReader::AbstractHttpMessageStreamReader(AbstractIODevice& device, size_t bufferSize) :
	_device(device),
	_buffer(bufferSize),
	_bufferSize(0),
	_bufferPosition(0),
	_parserAutoPtr()
{}

AbstractHttpMessageStreamReader::~AbstractHttpMessageStreamReader()
{}

const HttpMessageParser& AbstractHttpMessageStreamReader::parser() const
{
	return parserPrivate();
}

void AbstractHttpMessageStreamReader::reset()
{
	parserPrivate().reset();
	_buffer.clear();
	_bufferPosition = 0;
}

size_t AbstractHttpMessageStreamReader::read(char * bodyBuffer, size_t bodyBufferSize, const Timeout& timeout, size_t * bytesReadFromDevice)
{
	if (bytesReadFromDevice) {
		*bytesReadFromDevice = 0;
	}
	if (parserPrivate().isCompleted()) {
		parserPrivate().reset();
	}
	if (bodyBufferSize <= 0) {
		// TODO Maybe to throw an exception?
		return 0;
	}
	size_t bodyBytesRead = 0;
	// Parsing the rest of the data in the buffer
	while (_bufferPosition < _bufferSize) {
		char ch = _buffer[_bufferPosition++];
		if (parserPrivate().parse(ch)) {
			bodyBuffer[bodyBytesRead++] = ch;
		}
		if (parserPrivate().isCompleted() || parserPrivate().isBad() || bodyBytesRead >= bodyBufferSize) {
			return bodyBytesRead;
		}
	}
	// Reading available data from the device
	bool isFirstReading = true;
	while (true) {
		_bufferPosition = 0;
		_bufferSize = _device.read(&_buffer[0], _buffer.size(), isFirstReading ? timeout : Timeout());
		if (_bufferSize <= 0) {
			return bodyBytesRead;
		}
		isFirstReading = false;
		if (bytesReadFromDevice) {
			*bytesReadFromDevice += _bufferSize;
		}
		// Parsing data fetched from the device
		while (_bufferPosition < _bufferSize) {
			char ch = _buffer[_bufferPosition++];
			if (parserPrivate().parse(ch)) {
				bodyBuffer[bodyBytesRead++] = ch;
			}
			if (parserPrivate().isCompleted() || parserPrivate().isBad() || bodyBytesRead >= bodyBufferSize) {
				return bodyBytesRead;
			}
		}
	}
}

HttpMessageParser& AbstractHttpMessageStreamReader::parserPrivate() const
{
	if (!_parserAutoPtr.get()) {
		_parserAutoPtr.reset(createParser());
	}
	return *_parserAutoPtr.get();
}

} // namespace isl

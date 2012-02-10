#include <isl/AbstractIODevice.hxx>
#include <isl/Core.hxx>
#include <isl/Exception.hxx>
#include <isl/IOError.hxx>
#include <cstring>

#include <stdexcept>		// TODO Remove it

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractIODevice
------------------------------------------------------------------------------*/
	
AbstractIODevice::AbstractIODevice() :
	_isOpen(false),
	_readBuffer(),
	_readBufferPos(0),
	_ungetBuffer()
{
	_readBuffer.reserve(ReadBufferSize);
	_ungetBuffer.reserve(UngetBufferSize);
}

AbstractIODevice::~AbstractIODevice()
{}

void AbstractIODevice::open()
{
	if (_isOpen) {
		Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"I/O-device is already opened"));
		return;
	}
	openImplementation();
	_isOpen = true;
}

void AbstractIODevice::close()
{
	if (!_isOpen) {
		Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"I/O-device is already closed"));
		return;
	}
	openImplementation();
	_isOpen = false;
}

bool AbstractIODevice::getChar(char& ch, const Timeout& timeout)
{
	if (!_isOpen) {
		throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
	}
	if (_ungetBuffer.size() > 0) {
		ch = _ungetBuffer.back();
		_ungetBuffer.pop_back();
		return true;
	}
	if (_readBufferPos >= _readBuffer.size()) {
		// Read buffer has been expired
		readToReadBuffer(timeout);
	}
	if (_readBufferPos >= _readBuffer.size()) {
		// Read timeout has been expired
		return false;
	}
	ch = _readBuffer[_readBufferPos++];
	return true;
}

void AbstractIODevice::ungetChar(char ch)
{
	if (!_isOpen) {
		throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
	}
	if (_readBufferPos > 0) {
		--_readBufferPos;
	} else if (_ungetBuffer.size() >= UngetBufferSize) {
		// TODO Use 'isl::Exception' class instead
		throw std::runtime_error("I/O-device unget buffer overflow");
	} else {
		_ungetBuffer.push_back(ch);
	}
}

unsigned int AbstractIODevice::read(char * buffer, unsigned int bufferSize, const Timeout& timeout)
{
	if (!_isOpen) {
		throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
	}
	if (bufferSize <= 0) {
		return 0;
	}
	unsigned int bytesCopied = 0;
	// Utilizing the unget buffer first
	while ((_ungetBuffer.size() > 0) && (bytesCopied <= bufferSize)) {
		buffer[bytesCopied++] = _ungetBuffer.back();
		_ungetBuffer.pop_back();
	}
	// Utilizing the read buffer & read from I/O device if needed
	bool firstReading = true;
	while (bytesCopied <= bufferSize) {
		// Fill the read buffer if expired
		if (_readBufferPos >= _readBuffer.size()) {
			// Read buffer has been expired
			readToReadBuffer(firstReading ? timeout : Timeout());
			firstReading = false;
		}
		if (_readBuffer.size() <= 0) {
			// Read timeout has been expired
			break;
		}
		unsigned int bytesToCopy = std::min<unsigned int>(_readBuffer.size() - _readBufferPos, bufferSize - bytesCopied);
		memcpy(buffer + bytesCopied, &_readBuffer[_readBufferPos], bytesToCopy);
		_readBufferPos += bytesToCopy;
		bytesCopied += bytesToCopy;
	}
	return bytesCopied;
}

bool AbstractIODevice::putChar(char ch, const Timeout& timeout)
{
	return write(&ch, 1, timeout) == 1;
}

unsigned int AbstractIODevice::write(const char * buffer, unsigned int bufferSize, const Timeout& timeout)
{
	if (!_isOpen) {
		throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
	}
	if (bufferSize <= 0) {
		return 0;
	}
	return writeImplementation(buffer, bufferSize, timeout);
}

void AbstractIODevice::readToReadBuffer(const Timeout& timeout)
{
	_readBufferPos = 0;
	_readBuffer.resize(ReadBufferSize);
	_readBuffer.resize(readImplementation(&_readBuffer[0], ReadBufferSize, timeout));
}

} // namespace isl

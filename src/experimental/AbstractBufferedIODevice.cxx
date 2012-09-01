#include <isl/common.hxx>
#include <isl/AbstractBufferedIODevice.hxx>
#include <isl/LogMessage.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/IOError.hxx>
#include <cstring>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractBufferedIODevice
------------------------------------------------------------------------------*/
	
AbstractBufferedIODevice::AbstractBufferedIODevice() :
	_isOpen(false),
	_readBuffer(),
	_readBufferPos(0),
	_ungetBuffer()
{
	_readBuffer.reserve(ReadBufferSize);
	_ungetBuffer.reserve(UngetBufferSize);
}

AbstractBufferedIODevice::~AbstractBufferedIODevice()
{}

void AbstractBufferedIODevice::open()
{
	if (_isOpen) {
		warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, "I/O-device is already opened -> reopening the device"));
		close();
	}
	openImplementation();
	_isOpen = true;
}

void AbstractBufferedIODevice::close()
{
	if (!_isOpen) {
		warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, "I/O-device is already closed"));
		return;
	}
	closeImplementation();
	_readBuffer.clear();
	_readBufferPos = 0;
	_ungetBuffer.clear();
	_isOpen = false;
}

bool AbstractBufferedIODevice::getChar(char& ch, const Timeout& timeout)
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

void AbstractBufferedIODevice::ungetChar(char ch)
{
	if (!_isOpen) {
		throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
	}
	if (_readBufferPos > 0) {
		--_readBufferPos;
	} else if (_ungetBuffer.size() >= UngetBufferSize) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "I/O-device unget buffer overflow"));
	} else {
		_ungetBuffer.push_back(ch);
	}
}

size_t AbstractBufferedIODevice::read(char * buffer, size_t bufferSize, const Timeout& timeout)
{
	if (!_isOpen) {
		throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
	}
	if (bufferSize <= 0) {
		return 0;
	}
	size_t bytesCopied = 0;
	// Utilizing the unget buffer first
	while ((_ungetBuffer.size() > 0) && (bytesCopied <= bufferSize)) {
		buffer[bytesCopied++] = _ungetBuffer.back();
		_ungetBuffer.pop_back();
	}
	// Utilizing the read buffer & read from I/O device if needed
	while (bytesCopied <= bufferSize) {
		// Fill the read buffer if expired
		if (_readBufferPos >= _readBuffer.size()) {
			// Read buffer has been expired
			readToReadBuffer(timeout);
		}
		if (_readBuffer.size() <= 0) {
			// Read timeout has been expired
			break;
		}
		size_t bytesToCopy = std::min<size_t>(_readBuffer.size() - _readBufferPos, bufferSize - bytesCopied);
		memcpy(buffer + bytesCopied, &_readBuffer[_readBufferPos], bytesToCopy);
		_readBufferPos += bytesToCopy;
		bytesCopied += bytesToCopy;
	}
	return bytesCopied;
}

bool AbstractBufferedIODevice::putChar(char ch, const Timeout& timeout)
{
	return write(&ch, 1, timeout) == 1;
}

size_t AbstractBufferedIODevice::write(const char * buffer, size_t bufferSize, const Timeout& timeout)
{
	if (!_isOpen) {
		throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
	}
	if (bufferSize <= 0) {
		return 0;
	}
	return writeImplementation(buffer, bufferSize, timeout);
}

void AbstractBufferedIODevice::readToReadBuffer(const Timeout& timeout)
{
	_readBufferPos = 0;
	_readBuffer.resize(ReadBufferSize);
	_readBuffer.resize(readImplementation(&_readBuffer[0], ReadBufferSize, timeout));
}

} // namespace isl

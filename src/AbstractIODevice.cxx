#include <isl/common.hxx>
#include <isl/AbstractIODevice.hxx>
#include <isl/LogMessage.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
//#include <isl/IOError.hxx>
#include <cstring>

namespace isl
{

AbstractIODevice::AbstractIODevice() :
	_isOpen(false)
{}

AbstractIODevice::~AbstractIODevice()
{}

void AbstractIODevice::open()
{
	if (_isOpen) {
		warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, "I/O-device is already opened -> reopening the device"));
		close();
	}
	openImplementation();
	_isOpen = true;
}

void AbstractIODevice::close()
{
	if (!_isOpen) {
		warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, "I/O-device is already closed"));
		return;
	}
	closeImplementation();
	_isOpen = false;
}

size_t AbstractIODevice::read(char * buffer, size_t bufferSize, const Timeout& timeout)
{
	if (!_isOpen) {
		//throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
		throw Exception(NotOpenError(SOURCE_LOCATION_ARGS));
	}
	if (bufferSize <= 0) {
		return 0;
	}
	return readImplementation(buffer, bufferSize, timeout);
}

size_t AbstractIODevice::write(const char * buffer, size_t bufferSize, const Timeout& timeout)
{
	if (!_isOpen) {
		//throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::DeviceIsNotOpen));
		throw Exception(NotOpenError(SOURCE_LOCATION_ARGS));
	}
	if (bufferSize <= 0) {
		return 0;
	}
	return writeImplementation(buffer, bufferSize, timeout);
}

} // namespace isl

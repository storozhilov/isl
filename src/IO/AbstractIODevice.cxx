#include <isl/AbstractIODevice.hxx>
#include <isl/Core.hxx>
#include <isl/Exception.hxx>
#include <isl/IOError.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractIODevice
------------------------------------------------------------------------------*/
	
AbstractIODevice::AbstractIODevice() :
	_isOpen(false)
{}

AbstractIODevice::~AbstractIODevice()
{}

void AbstractIODevice::open()
{
	if (_isOpen) {
		Core::warningLog.logDebug(SOURCE_LOCATION_ARGS, L"I/O-device is already opened");
		return;
	}
	openImplementation();
	_isOpen = true;
}

void AbstractIODevice::close()
{
	if (!_isOpen) {
		Core::warningLog.logDebug(SOURCE_LOCATION_ARGS, L"I/O-device is already closed");
		return;
	}
	openImplementation();
	_isOpen = false;
}

unsigned int AbstractIODevice::read(char * buffer, unsigned int bufferSize, const Timeout& timeout)
{
	if (!_isOpen) {
		throw Exception(DeviceIsNotOpenIOError(SOURCE_LOCATION_ARGS));
	}
	return readImplementation(buffer, bufferSize, timeout);
}

unsigned int AbstractIODevice::write(const char * buffer, unsigned int bufferSize, const Timeout& timeout)
{
	if (!_isOpen) {
		throw Exception(DeviceIsNotOpenIOError(SOURCE_LOCATION_ARGS));
	}
	return writeImplementation(buffer, bufferSize, timeout);
}

} // namespace isl


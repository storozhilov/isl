#include <isl/AbstractLogDevice.hxx>
#include <isl/AbstractLogMessage.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractLogDevice
 *------------------------------------------------------------------------------*/

AbstractLogDevice::AbstractLogDevice() :
	_writeMutex()
{}

AbstractLogDevice::~AbstractLogDevice()
{}

void AbstractLogDevice::logMessage(const Log& log, const AbstractLogMessage& msg)
{
	MutexLocker locker(_writeMutex);
	writeMessage(log, msg);
}

} // namespace isl

#include <isl/AbstractLogDevice.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractLogDevice
 *------------------------------------------------------------------------------*/

AbstractLogDevice::AbstractLogDevice() :
	_writeMutex()
{}

//AbstractLogDevice::AbstractLogDevice() :
//	_writeMutex()
//{
//	_writeMutex.setForkable(true);
//}

AbstractLogDevice::~AbstractLogDevice()
{}

void AbstractLogDevice::logMessage(const std::wstring& prefix, const std::wstring &msg)
{
	MutexLocker locker(_writeMutex);
	writeMessage(prefix, msg);
}

} // namespace isl


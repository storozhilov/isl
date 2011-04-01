#include <isl/AbstractAsynchronousIODevice.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractAsynchronousIODevice
------------------------------------------------------------------------------*/

AbstractAsynchronousIODevice::AbstractAsynchronousIODevice() :
	AbstractIODevice(),
	_dataReadMutex(),
	_dataWriteMutex()
{}

Mutex& AbstractAsynchronousIODevice::dataReadMutex()
{
	return _dataReadMutex;
}

Mutex& AbstractAsynchronousIODevice::dataWriteMutex()
{
	return _dataWriteMutex;
}

} // namespace isl


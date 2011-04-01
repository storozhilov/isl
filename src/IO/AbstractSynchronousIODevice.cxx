#include <isl/AbstractSynchronousIODevice.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractSynchronousIODevice
------------------------------------------------------------------------------*/

AbstractSynchronousIODevice::AbstractSynchronousIODevice() :
	AbstractIODevice(),
	_dataTransferMutex()
{}

Mutex& AbstractSynchronousIODevice::dataTransferMutex()
{
	return _dataTransferMutex;
}

Mutex& AbstractSynchronousIODevice::dataReadMutex()
{
	return _dataTransferMutex;
}

Mutex& AbstractSynchronousIODevice::dataWriteMutex()
{
	return _dataTransferMutex;
}

} // namespace isl


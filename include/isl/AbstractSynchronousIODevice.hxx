#ifndef ISL__ABSTRACT_SYNCHRONOUS_IO_DEVICE__HXX
#define ISL__ABSTRACT_SYNCHRONOUS_IO_DEVICE__HXX

#include <isl/AbstractIODevice.hxx>
#include <isl/Mutex.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractSynchronousIODevice
------------------------------------------------------------------------------*/

class AbstractSynchronousIODevice : public AbstractIODevice
{
public:
	AbstractSynchronousIODevice();

	Mutex& dataTransferMutex();

	virtual Mutex& dataReadMutex();
	virtual Mutex& dataWriteMutex();
private:
	AbstractSynchronousIODevice(const AbstractSynchronousIODevice&);				// No copy

	AbstractSynchronousIODevice& operator=(const AbstractSynchronousIODevice&);			// No copy

	Mutex _dataTransferMutex;
};

} // namespace isl

#endif


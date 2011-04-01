#ifndef ISL__ABSTRACT_ASYNCHRONOUS_IO_DEVICE__HXX
#define ISL__ABSTRACT_ASYNCHRONOUS_IO_DEVICE__HXX

#include <isl/AbstractIODevice.hxx>
#include <isl/Mutex.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractAsynchronousIODevice
------------------------------------------------------------------------------*/

class AbstractAsynchronousIODevice : public AbstractIODevice
{
public:
	AbstractAsynchronousIODevice();

	virtual Mutex& dataReadMutex();
	virtual Mutex& dataWriteMutex();
private:
	AbstractAsynchronousIODevice(const AbstractAsynchronousIODevice&);				// No copy

	AbstractAsynchronousIODevice& operator=(const AbstractAsynchronousIODevice&);			// No copy

	Mutex _dataReadMutex;
	Mutex _dataWriteMutex;
};

} // namespace isl

#endif


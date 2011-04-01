#ifndef ISL__ABSTRACT_IO_DEVICE__HXX
#define ISL__ABSTRACT_IO_DEVICE__HXX

#include <isl/Timeout.hxx>
#include <isl/Mutex.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractIODevice
------------------------------------------------------------------------------*/

class AbstractIODevice
{
public:
	AbstractIODevice();
	virtual ~AbstractIODevice();

	void open();
	void close();
	inline bool isOpen() const
	{
		return _isOpen;
	}
	unsigned int read(char * buffer, unsigned int bufferSize, const Timeout& timeout = Timeout());
	unsigned int write(const char * buffer, unsigned int bufferSize, const Timeout& timeout = Timeout());

	virtual Mutex& dataReadMutex() = 0;
	virtual Mutex& dataWriteMutex() = 0;
protected:
	virtual void openImplementation() = 0;
	virtual void closeImplementation() = 0;
	virtual unsigned int readImplementation(char * buffer, unsigned int bufferSize, const Timeout& timeout) = 0;
	virtual unsigned int writeImplementation(const char * buffer, unsigned int bufferSize, const Timeout& timeout) = 0;

	bool _isOpen;
private:
	AbstractIODevice(const AbstractIODevice&);							// No copy

	AbstractIODevice& operator=(const AbstractIODevice&);						// No copy
};

} // namespace isl

#endif


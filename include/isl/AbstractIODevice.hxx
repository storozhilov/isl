#ifndef ISL__ABSTRACT_IO_DEVICE__HXX
#define ISL__ABSTRACT_IO_DEVICE__HXX

#include <isl/Timeout.hxx>
#include <isl/Mutex.hxx>
//#include <vector>
#include <sys/types.h>

namespace isl
{

//! I/O device abstraction
class AbstractIODevice
{
public:
	AbstractIODevice();
	virtual ~AbstractIODevice();

	//! Opens I/O device
	void open();
	//! Closes I/O device
	void close();
	//! Inspects if I/O device is open
	inline bool isOpen() const
	{
		return _isOpen;
	}
	//! Reads data to buffer from the I/O device
	/*!
	    \param buffer Pointer to buffer
	    \param bufferSize Buffer size
	    \param timeout Read timeout
	    \return Count of the actually received bytes
	*/
	size_t read(char * buffer, size_t bufferSize, const Timeout& timeout = Timeout());
	//! Writes data buffer to the I/O device
	/*!
	    \param buffer Pointer to the buffer
	    \param bufferSize Buffer size
	    \param timeout Write timeout
	    \return Count of the actually sent bytes
	*/
	size_t write(const char * buffer, size_t bufferSize, const Timeout& timeout = Timeout());
protected:
	//! Opening I/O device abstract method
	virtual void openImplementation() = 0;
	//! Closing I/O device abstract method
	virtual void closeImplementation() = 0;
	//! Reading from I/O device abstract method
	virtual size_t readImplementation(char * buffer, size_t bufferSize, const Timeout& timeout) = 0;
	//! Writing to I/O device abstract method
	virtual size_t writeImplementation(const char * buffer, size_t bufferSize, const Timeout& timeout) = 0;
	//! Sets is open flag to the new value
	inline void setIsOpen(bool newValue)
	{
		_isOpen = newValue;
	}
private:
	AbstractIODevice(const AbstractIODevice&);							// No copy

	AbstractIODevice& operator=(const AbstractIODevice&);						// No copy

	bool _isOpen;
};

} // namespace isl

#endif

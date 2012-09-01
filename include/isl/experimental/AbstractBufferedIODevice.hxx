#ifndef ISL__ABSTRACT_IO_DEVICE__HXX
#define ISL__ABSTRACT_IO_DEVICE__HXX

#include <isl/Timeout.hxx>
#include <isl/Mutex.hxx>
#include <vector>
#include <sys/types.h>

namespace isl
{

//! I/O device abstraction with buffered reading facility
/*!
  TODO Remove buffering
*/
class AbstractBufferedIODevice
{
public:
	AbstractBufferedIODevice();
	virtual ~AbstractBufferedIODevice();

	//! Opens I/O device
	void open();
	//! Closes I/O device
	void close();
	//! Inspects if I/O device is open
	inline bool isOpen() const
	{
		return _isOpen;
	}
	//! Reads one character from the I/O device
	/*!
	    \param ch Reference to the result character
	    \param timeout Read timeout
	    \return true if the character has been successfully read and false otherwise
	*/
	bool getChar(char& ch, const Timeout& timeout = Timeout());
	//! Pushes the character back to the I/O device for another reading
	/*!
	    \param ch Character to push back
	*/
	void ungetChar(char ch);
	//! Reads data to buffer from the I/O device
	/*!
	    \param buffer Pointer to buffer
	    \param bufferSize Buffer size
	    \param timeout Read timeout
	    \return Count of the actually received bytes
	*/
	size_t read(char * buffer, size_t bufferSize, const Timeout& timeout = Timeout());
	//! Writes character to the I/O device
 	/*!
	    \param ch Character to write
	    \param timeout Write timeout
	    \return true if character has been written to the I/O device or false otherwise
	*/
	bool putChar(char ch, const Timeout& timeout = Timeout());
	//! Writes buffered data to the I/O device
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

	bool _isOpen;
private:
	AbstractBufferedIODevice(const AbstractBufferedIODevice&);							// No copy

	AbstractBufferedIODevice& operator=(const AbstractBufferedIODevice&);						// No copy

	enum PrivateConstants {
#ifdef ISL__IO_DEVICE_READ_BUFFER_SIZE
		ReadBufferSize = ISL__IO_DEVICE_READ_BUFFER_SIZE,
#else
		ReadBufferSize = 1024,
#endif
#ifdef ISL__IO_DEVICE_UNGET_BUFFER_SIZE
		UngetBufferSize = ISL__IO_DEVICE_UNGET_BUFFER_SIZE
#else
		UngetBufferSize = 1024
#endif
	};

	void readToReadBuffer(const Timeout& timeout = Timeout());

	std::vector<char> _readBuffer;
	std::vector<char>::size_type _readBufferPos;
	std::vector<char> _ungetBuffer;
};

} // namespace isl

#endif

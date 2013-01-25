#ifndef ISL__ABSTRACT_POSIX_IO_DEVICE__HXX
#define ISL__ABSTRACT_POSIX_IO_DEVICE__HXX

#include <isl/AbstractIODevice.hxx>

namespace isl
{

//! POSIX I/O-device abstraction
class AbstractPosixIODevice : public AbstractIODevice_NEW
{
public:
	//! Device not open error class
	class NotOpenError : public AbstractError
	{
	public:
		//! Constructs device not open error
		/*!
		  \param SOURCE_LOCATION_ARGS_DECLARATION put SOURCE_LOCATION_ARGS macro here
		  \param info User info
		*/
		NotOpenError(SOURCE_LOCATION_ARGS_DECLARATION, const std::string& info = std::string()) :
			AbstractError(SOURCE_LOCATION_ARGS_PASSTHRU, info)
		{}
		//! Clones error
		virtual AbstractError * clone() const
		{
			return new NotOpenError(*this);
		}
	private:
		NotOpenError();

		virtual std::string composeMessage() const
		{
			return "I/O-device is not open";
		}
	};

	AbstractPosixIODevice();
	AbstractPosixIODevice(int handle);
	virtual ~AbstractPosixIODevice();

	//! Returns I/O device handle
	inline int handle() const
	{
		return _handle;
	}
	//! Inspects I/O device to be open
	inline bool isOpen() const
	{
		return _isOpen;
	}
	//! Opens I/O device
	void open();
	//! Closes I/O device
	void close();
	//! Reads data to buffer from the I/O device
	/*!
	    \param buffer Pointer to buffer
	    \param bufferSize Buffer size
	    \param timeout Read timeout
	    \return Count of the actually received bytes
	*/
	virtual size_t read(char * buffer, size_t bufferSize, const Timeout& timeout = Timeout());
	//! Writes data buffer to the I/O device
	/*!
	    \param buffer Pointer to the buffer
	    \param bufferSize Buffer size
	    \param timeout Write timeout
	    \return Count of the actually sent bytes
	*/
	virtual size_t write(const char * buffer, size_t bufferSize, const Timeout& timeout = Timeout());
protected:
	//! Opens a POSIX I/O device
	/*!
	  \return POSIX I/O device descriptor
	*/
	virtual int openImpl() = 0;
	virtual void onReadException() = 0;
	virtual void onReadEndOfFile() = 0;
	virtual void onWriteException() = 0;
	virtual void onWriteEndOfFile() = 0;
private:
	void close(bool raiseExceptionOnError);

	int _handle;
	bool _isOpen;
};

} // namespace isl

#endif

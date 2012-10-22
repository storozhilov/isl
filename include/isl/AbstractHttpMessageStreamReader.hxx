#ifndef ISL__ABSTRACT_HTTP_MESSAGE_STREAM_READER__HXX
#define ISL__ABSTRACT_HTTP_MESSAGE_STREAM_READER__HXX

#include <isl/AbstractIODevice.hxx>
#include <isl/HttpMessageParser.hxx>
#include <vector>
#include <memory>

#ifndef ISL__ABSTRACT_HTTP_MESSAGE_STREAM_READER_DEFAULT_BUFFER_SIZE
#define ISL__ABSTRACT_HTTP_MESSAGE_STREAM_READER_DEFAULT_BUFFER_SIZE 4096	// 4 Kb
#endif

namespace isl
{

//! Base abstract class for HTTP-message stream readers
class AbstractHttpMessageStreamReader
{
public:
	//! Class constants
	enum Constants {
		DefaultBufferSize = ISL__ABSTRACT_HTTP_MESSAGE_STREAM_READER_DEFAULT_BUFFER_SIZE
	};
	//! Constructs reader
	/*!
	  \param device Reference to the I/O-device to fetch data from
	  \param bufferSize Data reading buffer size,
	*/
	AbstractHttpMessageStreamReader(AbstractIODevice& device, size_t bufferSize = DefaultBufferSize);
	virtual ~AbstractHttpMessageStreamReader();
	//! Returns a constant reference to parser
	const HttpMessageParser& parser() const;
	//! Resets reader
	virtual void reset();
	//! Reads an HTTP-message from the device and puts the HTTP-message body to the supplied buffer
	/*!
	  \param bodyBuffer Buffer to put the message body to
	  \param bodyBufferSize Body buffer size
	  \param timeout Timeout for reading operation
	  \param bytesReadFromDevice Pointer to memory location where number of bytes have been fetched from the device is to be put
	  \return Number of body bytes have been put in the body buffer
	*/
	size_t read(char * bodyBuffer, size_t bodyBufferSize, const Timeout& timeout = Timeout::defaultTimeout(), size_t * bytesReadFromDevice = 0);
	//! Returns unparsed bytes amount in reading buffer
	inline size_t bytesAvailable() const
	{
		return (_bufferSize <= 0) ? 0 : _bufferSize - _bufferPosition - 1;
	}
protected:
	//! Parser creation factory method
	virtual HttpMessageParser * createParser() const = 0;
private:
	AbstractHttpMessageStreamReader();

	//! Returns a reference to parser
	HttpMessageParser& parserPrivate() const;

	AbstractIODevice& _device;
	std::vector<char> _buffer;
	size_t _bufferSize;
	size_t _bufferPosition;
	mutable std::auto_ptr<HttpMessageParser> _parserAutoPtr;
};

} // namespace isl

#endif

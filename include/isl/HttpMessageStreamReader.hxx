#ifndef ISL__HTTP_MESSAGE_STREAM_READER__HXX
#define ISL__HTTP_MESSAGE_STREAM_READER__HXX

#include <isl/AbstractIODevice.hxx>
#include <isl/HttpMessageParser.hxx>
#include <isl/Timestamp.hxx>
#include <vector>

#ifndef ISL__HTTP_MESSAGE_STREAM_READER_DEFAULT_BUFFER_SIZE
#define ISL__HTTP_MESSAGE_STREAM_READER_DEFAULT_BUFFER_SIZE 4096	// 4 Kb
#endif

namespace isl
{

//! HTTP-message reader
class HttpMessageStreamReader
{
public:
	//! Class constants
	enum Constants {
		DefaultBufferSize = ISL__HTTP_MESSAGE_STREAM_READER_DEFAULT_BUFFER_SIZE
	};
	//! Constructs an HTTP-message reader
	/*!
	  \param parser Reference to the HTTP-message parser
	  \param bufferSize Read data buffer size
	*/
	HttpMessageStreamReader(HttpMessageParser& parser, size_t bufferSize = DefaultBufferSize);
	virtual ~HttpMessageStreamReader();

	//! Returns a reference to the HTTP-message parser
	inline HttpMessageParser& parser()
	{
		return _parser;
	}
	//! Resets reader to it's initial state
	virtual void reset();
	//! Fetches a complete ot partial HTTP-message
	/*!
	  Fetches HTTP-message until complete or bad HTTP-message has been fetched or when the
	  supplied body buffer has no space for the body of the HTTP-message.
	  \param device Reference to the I/O-device to fetch data from
	  \param limit Read data limit timestamp
	  \param bodyBuffer Pointer to buffer where HTTP-message body should be put
	  \param bodyBufferSize HTTP-message body buffer size
	  \param bytesReadFromDevice Pointer to memory location where the number of bytes have been fetched from the device is to be put
	  \return A flag if complete HTTP-message has been received and the amount of bytes, which has been stored in body buffer
	*/
	virtual std::pair<bool, size_t> read(AbstractIODevice& device, const Timestamp& limit, char * bodyBuffer, size_t bodyBufferSize, size_t * bytesReadFromDevice = 0);
private:
	HttpMessageParser& _parser;
	const size_t _bufferSize;
	std::vector<char> _readBuffer;
	size_t _bytesRead;
	size_t _bytesParsed;
};

} // namespace isl

#endif

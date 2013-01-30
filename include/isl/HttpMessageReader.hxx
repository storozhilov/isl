#ifndef ISL__HTTP_MESSAGE_READER__HXX
#define ISL__HTTP_MESSAGE_READER__HXX

#include <isl/HttpMessageParser.hxx>
#include <isl/AbstractIODevice.hxx>
#include <isl/Timestamp.hxx>
#include <vector>

#ifndef ISL__HTTP_MESSAGE_READER_DEFAULT_MAX_BODY_SIZE
#define ISL__HTTP_MESSAGE_READER_DEFAULT_MAX_BODY_SIZE 102400		// 100 Kb
#endif

#ifndef ISL__HTTP_MESSAGE_READER_DEFAULT_BUFFER_SIZE
#define ISL__HTTP_MESSAGE_READER_DEFAULT_BUFFER_SIZE 4096		// 4 Kb
#endif

namespace isl
{

//! HTTP-message reader
class HttpMessageReader
{
public:
	//! Class constants
	enum Constants {
		DefaultMaxBodySize = ISL__HTTP_MESSAGE_READER_DEFAULT_MAX_BODY_SIZE,
		DefaultBufferSize = ISL__HTTP_MESSAGE_READER_DEFAULT_BUFFER_SIZE
	};
	//! Constructs an HTTP-message reader
	/*!
	  \param parser Reference to the HTTP-message parser
	  \param maxBodySize Maximum body size
	  \param bufferSize Read data buffer size
	*/
	HttpMessageReader(HttpMessageParser& parser, size_t maxBodySize = DefaultMaxBodySize, size_t bufferSize = DefaultBufferSize);
	virtual ~HttpMessageReader();

	//! Returns a reference to the HTTP-message parser
	inline HttpMessageParser& parser()
	{
		return _parser;
	}
	//! Returns a constant reference to the HTTP-message body
	inline const std::string& body() const
	{
		return _body;
	}
	//! Resets reader to it's initial state
	virtual void reset();
	//! Fetches a request
	/*!
	  \param device Reference to the I/O-device to fetch data from
	  \param limit Read data limit timestamp
	  \param bytesReadFromDevice Pointer to memory location where number of bytes have been fetched from the device is to be put
	  \return TRUE if the complete HTTP-message has been received or FALSE otherwise
	  \note This method throws an Exception if the message body is too long or HTTP-message parser is already in error state.
	*/
	virtual bool read(AbstractIODevice& device, const Timestamp& limit, size_t * bytesReadFromDevice = 0);
private:
	HttpMessageParser& _parser;
	const size_t _maxBodySize;
	const size_t _bufferSize;
	std::vector<char> _readBuffer;
	size_t _bytesRead;
	size_t _bytesParsed;
	std::vector<char> _bodyBuffer;
	std::string _body;
};

} // namespace isl

#endif

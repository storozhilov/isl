#ifndef ISL__HTTP_REQUEST_STREAM_READER__HXX
#define ISL__HTTP_REQUEST_STREAM_READER__HXX

#include <isl/AbstractHttpMessageStreamReader.hxx>

#ifndef ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_METHOD_LENGTH
#define ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_METHOD_LENGTH 20
#endif
#ifndef ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_URI_LENGTH
#define ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_URI_LENGTH 4096
#endif
#ifndef ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_VERSION_LENGTH
#define ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_VERSION_LENGTH 20
#endif

namespace isl
{

//! HTTP-request stream reader
class HttpRequestStreamReader : public AbstractHttpMessageStreamReader
{
public:
	enum Constants {
		DefaultMaxMethodLength = ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_METHOD_LENGTH,
		DefaultMaxUriLength = ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_URI_LENGTH,
		DefaultMaxVersionLength = ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_VERSION_LENGTH
	};
	//! Constructs HTTP-request stream reader
	/*!
	  \param device Reference to the I/O-device to fetch data from
	  \param bufferSize Data reading buffer size
	  \param maxMethodLength Maximum HTTP-method length
	  \param maxUriLength Maximum URI length
	  \param maxVersionLength Maximum HTTP-version length
	*/
	HttpRequestStreamReader(AbstractIODevice& device, size_t bufferSize = DefaultBufferSize,
			size_t maxMethodLength = DefaultMaxMethodLength, size_t maxUriLength = DefaultMaxUriLength,
			size_t maxVersionLength = DefaultMaxVersionLength);
	//! Returns HTTP-method
	inline const std::string& method() const
	{
		return parser().firstToken();
	}
	//! Returns URI
	inline const std::string& uri() const
	{
		return parser().secondToken();
	}
	//! Returns HTTP-version
	inline const std::string& version() const
	{
		return parser().thirdToken();
	}
protected:
	//! Parser creation factory method
	virtual HttpMessageParser * createParser() const;
private:
	const size_t _maxMethodLength;
	const size_t _maxUriLength;
	const size_t _maxVersionLength;
};

} // namespace isl

#endif

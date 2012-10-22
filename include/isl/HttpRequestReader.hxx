#ifndef ISL__HTTP_REQUEST_READER__HXX
#define ISL__HTTP_REQUEST_READER__HXX

#include <isl/HttpRequestStreamReader.hxx>
#include <vector>

namespace isl
{

#ifndef ISL__HTTP_REQUEST_READER_DEFAULT_MAX_BODY_SIZE
#define ISL__HTTP_REQUEST_READER_DEFAULT_MAX_BODY_SIZE 102400		// 100 Kb
#endif

#ifndef ISL__HTTP_REQUEST_READER_DEFAULT_BUFFER_SIZE
#define ISL__HTTP_REQUEST_READER_DEFAULT_BUFFER_SIZE 1024		// 1 Kb
#endif

//! HTTP-request reader
/*!
  TODO File upload support
*/
class HttpRequestReader
{
public:
	//! Class constants
	enum Constants {
		DefaultMaxBodySize = ISL__HTTP_REQUEST_READER_DEFAULT_MAX_BODY_SIZE,
		DefaultBufferSize = ISL__HTTP_REQUEST_READER_DEFAULT_BUFFER_SIZE,
		BufferSize = 1024
	};
	//! Constructs a reader
	/*!
	  \param device Reference to the I/O-device to fetch data from
	  \param maxBodySize Maximum body size
	  \param bufferSize Boby buffer size
	  \param streamReaderBufferSize Internal stream reader buffer size
	*/
	HttpRequestReader(AbstractIODevice& device, size_t maxBodySize = DefaultMaxBodySize, size_t bufferSize = DefaultBufferSize,
			size_t streamReaderBufferSize = HttpRequestStreamReader::DefaultBufferSize);
	//! Returns const reference to the internal stream reader
	inline const HttpRequestStreamReader& streamReader() const
	{
		return _streamReader;
	}
	//! Returns a path part of the URI of the HTTP-request
	inline const std::string& path() const
	{
		return _path;
	}
	//! Returns a query part of the URI of the HTTP-request
	inline const std::string& query() const
	{
		return _query;
	}
	//! Returns a constant reference to the HTTP-cookies of the HTTP-request
	const Http::RequestCookies& cookies() const;
	//! Returns a constant reference to the body of the HTTP-request
	inline const std::string& body() const
	{
		return _body;
	}
	//! Returns a constant reference to the GET parameters of the HTTP-request
	const Http::Params& get() const;
	//! Returns a constant reference to the POST parameters of the HTTP-request
	const Http::Params& post() const;
	//! Resets reader to it's initial state
	void reset();
	//! Fetches a request
	/*!
	  \note This method throws an Exception if the message body is too long.

	  \param timeout Read data timeout
	  \param bytesReadFromDevice Pointer to memory location where number of bytes have been fetched from the device is to be put
	  \return TRUE if the complete HTTP-request has been received or FALSE otherwise
	*/
	bool receive(Timeout timeout = Timeout::defaultTimeout(), size_t * bytesReadFromDevice = 0);
private:
	HttpRequestReader();

	HttpRequestStreamReader _streamReader;
	size_t _maxBodySize;
	std::vector<char> _buffer;
	std::string _path;
	std::string _query;
	std::string _body;
	mutable Http::Params _get;
	mutable bool _getExtracted;
	mutable Http::Params _post;
	mutable bool _postExtracted;
	mutable Http::RequestCookies _cookies;
	mutable bool _cookiesExtracted;
};

} // namespace isl

#endif

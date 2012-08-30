#ifndef ISL__HTTP_REQUEST_READER__HXX
#define ISL__HTTP_REQUEST_READER__HXX

#include <isl/HttpRequestStreamReader.hxx>

namespace isl
{

//! HTTP-request reader
/*!
  TODO File upload support
*/
class HttpRequestReader
{
private:
	enum PrivateConstants {
		DefaultMaxBodySize = 102400,			// 100 Kb
		//DefaultTimeoutSec = 1,
		BufferSize = 1024
	};
public:
	//! Constructs a reader
	/*!
	  \param device Reference to the I/O-device to fetch data from
	*/
	HttpRequestReader(AbstractIODevice& device, size_t maxBodySize = DefaultMaxBodySize);

	inline bool isCompleted() const
	{
		return _streamReader.isCompleted();
	}
	inline bool isBad() const
	{
		return _streamReader.isBad();
	}
	inline std::string error() const
	{
		return _streamReader.parsingError();
	}
	//! Returns an HTTP-method of the HTTP-request
	inline std::string method() const
	{
		return _streamReader.method();
	}
	//! Returns a URI of the HTTP-request
	inline std::string uri() const
	{
		return _streamReader.uri();
	}
	//! Returns an HTTP-version of the HTTP-request
	inline std::string version() const
	{
		return _streamReader.version();
	}
	//! Returns a path part of the URI of the HTTP-request
	inline std::string path() const
	{
		return _path;
	}
	//! Returns a query part of the URI of the HTTP-request
	inline std::string query() const
	{
		return _query;
	}
	//! Returns a constant reference to the HTTP-header of the HTTP-request
	inline const Http::Params& header() const
	{
		return _streamReader.header();
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
	//! Fetches a request (Obsoleted!)
	/*!
	  \param timeout Read data timeout
	  \param maxBodySize Maximum body size
	*/
	//void receive(Timeout timeout = Timeout(DefaultTimeoutSec), size_t maxBodySize = DefaultMaxBodySize);
	//! Fetches a request
	/*!
	  \param timeout Read data timeout
	  \return TRUE if the complete HTTP-request has been received or false if the timeout has bee expired or an error occured
	*/
	bool receive(Timeout timeout = Timeout::defaultTimeout());
private:
	HttpRequestReader();

	HttpRequestStreamReader _streamReader;
	size_t _maxBodySize;
	char _buf[BufferSize];
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

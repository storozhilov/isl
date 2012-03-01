#ifndef ISL__HTTP_REQUEST_READER__HXX
#define ISL__HTTP_REQUEST_READER__HXX

#include <isl/HttpRequestStreamReader.hxx>

namespace isl
{

class HttpRequestReader
{
	// TODO File upload support
private:
	enum PrivateConstants {
		DefaultMaxBodySize = 102400,			// 100 Kb
		DefaultTimeoutSec = 1,
		BufferSize = 1024
	};
public:
	HttpRequestReader(AbstractIODevice& device);

	inline std::string method() const
	{
		return _streamReader.method();
	}
	inline std::string uri() const
	{
		return _streamReader.uri();
	}
	inline std::string version() const
	{
		return _streamReader.version();
	}
	inline std::string path() const
	{
		return _path;
	}
	inline std::string query() const
	{
		return _query;
	}
	inline const Http::Header& header() const
	{
		return _streamReader.header();
	}
	//! Inspects HTTP-header for specified field name
	/*!
	  \param fieldName Name of the HTTP-header field.
	  \return TRUE if HTTP-header field exists.
	*/
	inline bool headerContains(const std::string &fieldName) const
	{
		return _streamReader.headerContains(fieldName);
	}
	//! Inspects HTTP-header for specified field name and value
	/*!
	  \param fieldName Name of the HTTP-header field.
	  \param fieldValue Value of the HTTP-header field.
	  \return TRUE if HTTP-header field exists.
	*/
	bool headerContains(const std::string &fieldName, const std::string &fieldValue) const
	{
		return _streamReader.headerContains(fieldName, fieldValue);
	}
	//! Returns first occurence of HTTP-header value of the specified field
	/*!
	  \param fieldName Name of the HTTP-header field.
	  \return Value of the HTTP-header field or empty string if it does not exists
	*/
	std::string header(const std::string &fieldName) const
	{
		return _streamReader.header(fieldName);
	}
	//! Returns all occurences of the HTTP-header values list of the specified field
	/*!
	  \param fieldName Name of the HTTP-header field.
	  \return Values list of the HTTP-header field or empty list if it does not exists
	*/
	std::list<std::string> headers(const std::string &fieldName) const
	{
		return _streamReader.headers(fieldName);
	}
	inline const Http::RequestCookies& cookies() const
	{
		return _streamReader.cookies();
	}
	/*inline std::string cookie(const std::string& cookieName) const
	{
		return _streamReader.cookie(cookieName);
	}*/
	inline std::string body() const
	{
		return _body;
	}
	inline std::string get(const std::string& paramName) const
	{
		Http::Params::const_iterator pos = _get.find(paramName);
		return pos == _get.end() ? std::string() : pos->second;
	}
	inline const Http::Params& get() const
	{
		return _get;
	}
	inline std::string post(const std::string& paramName) const
	{
		Http::Params::const_iterator pos = _post.find(paramName);
		return pos == _post.end() ? std::string() : pos->second;
	}
	inline const Http::Params& post() const
	{
		return _post;
	}
	void reset();
	void receive(Timeout timeout = Timeout(DefaultTimeoutSec), unsigned int maxBodySize = DefaultMaxBodySize);
private:
	HttpRequestReader();

	HttpRequestStreamReader _streamReader;
	std::string _path;
	std::string _query;
	std::string _body;
	Http::Params _get;
	Http::Params _post;
};

} // namespace isl

#endif

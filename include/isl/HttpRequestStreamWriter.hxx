#ifndef ISL__HTTP_REQUEST_STREAM_WRITER__HXX
#define ISL__HTTP_REQUEST_STREAM_WRITER__HXX

#include <isl/HttpMessageStreamWriter.hxx>

namespace isl
{

//! HTTP-request stream writer
class HttpRequestStreamWriter : public HttpMessageStreamWriter
{
public:
	//! Constructs HTTP-request stream writer
	/*!
	  \param device Reference to the I/O-device to write data to
	  \param uri Request URI
	  \param method HTTP-method
	  \param version HTTP-version
	*/
	HttpRequestStreamWriter(AbstractIODevice& device, const std::string& uri, const std::string& method = std::string("GET"), const std::string& version = std::string("HTTP/1.1")) :
		HttpMessageStreamWriter(device),
		_method(method),
		_uri(uri),
		_version(version)
	{}
	//! Returns HTTP-method of the request
	inline std::string method() const
	{
		return _method;
	}
	//! Returns URI of the request
	inline std::string uri() const
	{
		return _uri;
	}
	//! Returns HTTP-version of the request
	inline std::string version() const
	{
		return _version;
	}
	//! Resets HTTP-request stream writer
	/*!
	  \param uri Request URI
	*/
	inline void reset(const std::string& uri)
	{
		HttpMessageStreamWriter::reset();
		_uri = uri;
	}
	//! Resets HTTP-request stream writer
	/*!
	  \param uri Request URI
	  \param method HTTP-method
	*/
	inline void reset(const std::string& uri, const std::string& method)
	{
		HttpMessageStreamWriter::reset();
		_uri = uri;
		_method = method;
	}
	//! Resets HTTP-request stream writer
	/*!
	  \param uri Request URI
	  \param method HTTP-method
	  \param version HTTP-version
	*/
	inline void reset(const std::string& uri, const std::string& method, const std::string& version)
	{
		HttpMessageStreamWriter::reset();
		_uri = uri;
		_method = method;
		_version = version;
	}
private:
	HttpRequestStreamWriter();

	virtual std::string composeFirstLine() const
	{
		std::string firstLine(_method);
		firstLine.append(1, ' ');
		firstLine.append(_uri);
		firstLine.append(1, ' ');
		firstLine.append(_version);
		firstLine.append("\r\n");
		return firstLine;
	}

	std::string _method;
	std::string _uri;
	std::string _version;
};

} // namespace isl

#endif

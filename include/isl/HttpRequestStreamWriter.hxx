#ifndef ISL__HTTP_REQUEST_STREAM_WRITER__HXX
#define ISL__HTTP_REQUEST_STREAM_WRITER__HXX

#include <isl/HttpMessageStreamWriter.hxx>

namespace isl
{

class HttpRequestStreamWriter : public HttpMessageStreamWriter
{
public:
	HttpRequestStreamWriter(AbstractIODevice& device, const std::string& uri, const std::string& method = std::string("GET"), const std::string& version = std::string("HTTP/1.1")) :
		HttpMessageStreamWriter(device),
		_uri(uri),
		_method(method),
		_version(version)
	{}

	inline std::string method() const
	{
		return _method;
	}
	inline std::string uri() const
	{
		return _uri;
	}
	inline std::string version() const
	{
		return _version;
	}
	inline void reset(const std::string& uri)
	{
		HttpMessageStreamWriter::reset();
		_uri = uri;
	}
	inline void reset(const std::string& uri, const std::string& method)
	{
		HttpMessageStreamWriter::reset();
		_uri = uri;
		_method = method;
	}
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
		firstLine.append(String::urlEncode(_uri));
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

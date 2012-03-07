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
	inline const Http::Params& header() const
	{
		return _streamReader.header();
	}
	const Http::RequestCookies& cookies() const;
	inline const std::string& body() const
	{
		return _body;
	}
	const Http::Params& get() const;
	const Http::Params& post() const;
	void reset();
	void receive(Timeout timeout = Timeout(DefaultTimeoutSec), size_t maxBodySize = DefaultMaxBodySize);
private:
	HttpRequestReader();

	HttpRequestStreamReader _streamReader;
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

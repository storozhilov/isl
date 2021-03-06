#ifndef ISL__HTTP_REQUEST_READER__HXX
#define ISL__HTTP_REQUEST_READER__HXX

#include <isl/HttpMessageReader.hxx>
#include <isl/HttpRequestParser.hxx>

namespace isl
{

//! HTTP-request reader
class HttpRequestReader : public HttpMessageReader
{
public:
	//! Constructs an HTTP-request reader
	/*!
	  \param parser Reference to the HTTP-request parser
	  \param maxBodySize Maximum body size
	  \param bufferSize Read data buffer size
	*/
	HttpRequestReader(HttpRequestParser& parser, size_t maxBodySize = DefaultMaxBodySize, size_t bufferSize = DefaultBufferSize);
	//! Returns a reference to the HTTP-request parser
	inline HttpRequestParser& parser()
	{
		return _parser;
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
	//! Returns a constant reference to the GET parameters of the HTTP-request
	const Http::Params& get() const;
	//! Returns a constant reference to the POST parameters of the HTTP-request
	const Http::Params& post() const;
	//! Resets reader to it's initial state
	virtual void reset();
protected:
	virtual void onNewMessage();
	virtual void onCompleteMessage();
private:
	HttpRequestReader();

	HttpRequestParser& _parser;
	std::string _path;
	std::string _query;
	mutable Http::Params _get;
	mutable bool _getExtracted;
	mutable Http::Params _post;
	mutable bool _postExtracted;
	mutable Http::RequestCookies _cookies;
	mutable bool _cookiesExtracted;
};

} // namespace isl

#endif

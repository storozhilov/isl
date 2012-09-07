#ifndef ISL__HTTP_REQUEST_STREAM_READER__HXX
#define ISL__HTTP_REQUEST_STREAM_READER__HXX

#include <isl/HttpMessageStreamReader.hxx>
#include <isl/HttpRequestCookieParser.hxx>

namespace isl
{

//! HTTP-request stream reader
/*!
  TODO Fetch cookies method
*/
class HttpRequestStreamReader : public HttpMessageStreamReader
{
public:
	//! Constructs HTTP-request stream reader
	/*!
	  \param device Reference to the I/O-device to fetch data from
	  \param bufferSize Data reading buffer size
	*/
	HttpRequestStreamReader(AbstractIODevice& device, size_t bufferSize = DefaultBufferSize) :
		HttpMessageStreamReader(device, bufferSize),
		_method(),
		_uri(),
		_version(),
		_maxMethodLength(MaxMethodLength),
		_maxUriLength(MaxUriLength),
		_maxVersionLength(MaxVersionLength)
	{}
	//! Returns HTTP-method of the HTTP-request
	inline std::string method() const
	{
		return _method;
	}
	//! Returns URI of the HTTP-request
	inline std::string uri() const
	{
		return _uri;
	}
	//! Returns HTTP-version of the HTTP-request
	inline std::string version() const
	{
		return _version;
	}
	//! Returns maximum HTTP-method length
	inline size_t maxMethodLength() const
	{
		return _maxMethodLength;
	}
	//! Sets maximum HTTP-method length
	/*!
	  \param newValue New maximum HTTP-method length value
	*/
	inline void setMaxMethodLength(size_t newValue)
	{
		_maxMethodLength = newValue;
	}
	//! Returns maximum URI length
	inline size_t maxUriLength() const
	{
		return _maxUriLength;
	}
	//! Sets maximum URI length
	/*!
	  \param newValue New maximum URI length value
	*/
	inline void setMaxUriLength(size_t newValue)
	{
		_maxUriLength = newValue;
	}
	//! Returns maximum HTTP-version length
	inline size_t maxVersionLength() const
	{
		return _maxVersionLength;
	}
	//! Sets maximum HTTP-version length
	/*!
	  \param newValue New maximum HTTP-version length value
	*/
	inline void setMaxVersionLength(size_t newValue)
	{
		_maxVersionLength = newValue;
	}
	//! Resets HTTP-request stream reader to it's initial state
	virtual void reset()
	{
		HttpMessageStreamReader::reset();
		_method.clear();
		_uri.clear();
		_version.clear();
	}
private:
	HttpRequestStreamReader();

	enum PrivateConstants {
		MaxMethodLength = 20,
		MaxUriLength = 4096,
		MaxVersionLength = 20
	};
	
	virtual bool isAllowedInFirstToken(char ch) const
	{
		return Http::isToken(ch);
	}
	virtual void appendToFirstToken(char ch)
	{
		if (_method.length() >= _maxMethodLength) {
			setIsBad(Error(SOURCE_LOCATION_ARGS, "Request method is too long"));
			return;
		}
		_method.append(1, ch);
	}
	virtual bool isAllowedInSecondToken(char ch) const
	{
		return Http::isAllowedInUri(ch);
	}
	virtual void appendToSecondToken(char ch)
	{
		if (_uri.length() >= _maxUriLength) {
			setIsBad(Error(SOURCE_LOCATION_ARGS, "Request URI is too long"));
			return;
		}
		_uri.append(1, ch);
	}
	virtual bool isAllowedInThirdToken(char ch) const
	{
		return Http::isAllowedInVersion(ch);
	}
	virtual void appendToThirdToken(char ch)
	{
		if (_version.length() >= _maxVersionLength) {
			setIsBad(Error(SOURCE_LOCATION_ARGS, "Request version is too long"));
			return;
		}
		_version.append(1, ch);
	}

	std::string _method;
	std::string _uri;
	std::string _version;
	size_t _maxMethodLength;
	size_t _maxUriLength;
	size_t _maxVersionLength;
};

} // namespace isl

#endif

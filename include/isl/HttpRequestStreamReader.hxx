#ifndef ISL__HTTP_REQUEST_STREAM_READER__HXX
#define ISL__HTTP_REQUEST_STREAM_READER__HXX

#include <isl/HttpMessageStreamReader.hxx>
#include <isl/HttpRequestCookieParser.hxx>

namespace isl
{

class HttpRequestStreamReader : public HttpMessageStreamReader
{
public:
	HttpRequestStreamReader(AbstractIODevice& device) :
		HttpMessageStreamReader(device),
		_method(),
		_uri(),
		_version(),
		_maxMethodLength(MaxMethodLength),
		_maxUriLength(MaxUriLength),
		_maxVersionLength(MaxVersionLength)
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
	inline size_t maxMethodLength() const
	{
		return _maxMethodLength;
	}
	inline void setMaxMethodLength(size_t newValue)
	{
		_maxMethodLength = newValue;
	}
	inline size_t maxUriLength() const
	{
		return _maxUriLength;
	}
	inline void setMaxUriLength(size_t newValue)
	{
		_maxUriLength = newValue;
	}
	inline size_t maxVersionLength() const
	{
		return _maxVersionLength;
	}
	inline void setMaxVersionLength(size_t newValue)
	{
		_maxVersionLength = newValue;
	}

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
			setIsBad("Request method is too long");
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
			setIsBad("Request URI is too long");
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
			setIsBad("Request version is too long");
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

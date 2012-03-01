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
		_cookies(),
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
	inline const Http::RequestCookies& cookies() const
	{
		return _cookies;
	}
	inline unsigned int maxMethodLength() const
	{
		return _maxMethodLength;
	}
	inline void setMaxMethodLength(unsigned int newValue)
	{
		_maxMethodLength = newValue;
	}
	inline unsigned int maxUriLength() const
	{
		return _maxUriLength;
	}
	inline void setMaxUriLength(unsigned int newValue)
	{
		_maxUriLength = newValue;
	}
	inline unsigned int maxVersionLength() const
	{
		return _maxVersionLength;
	}
	inline void setMaxVersionLength(unsigned int newValue)
	{
		_maxVersionLength = newValue;
	}

	virtual void reset()
	{
		HttpMessageStreamReader::reset();
		_method.clear();
		_uri.clear();
		_version.clear();
		_cookies.clear();
	}
private:
	HttpRequestStreamReader();

	enum PrivateConstants {
		MaxMethodLength = 20,
		MaxUriLength = 4096,
		MaxVersionLength = 20
	};
	
	virtual void onHeaderAppended(const std::string& fieldName, const std::string& fieldValue)
	{
		if (fieldName == "Cookie") {
			// Parsing and adding cookies
			HttpRequestCookieParser cookieParser;
			Http::RequestCookies cookies = cookieParser.parse(fieldValue);
			_cookies.insert(cookies.begin(), cookies.end());
		}
	}
	virtual bool isAllowedInFirstToken(char ch) const
	{
		return Http::isToken(ch);
	}
	virtual void appendToFirstToken(char ch)
	{
		if (_method.length() >= _maxMethodLength) {
			setIsBad(L"Request method is too long");
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
			setIsBad(L"Request URI is too long");
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
			setIsBad(L"Request version is too long");
			return;
		}
		_version.append(1, ch);
	}

	std::string _method;
	std::string _uri;
	std::string _version;
	Http::RequestCookies _cookies;
	unsigned int _maxMethodLength;
	unsigned int _maxUriLength;
	unsigned int _maxVersionLength;
};

} // namespace isl

#endif

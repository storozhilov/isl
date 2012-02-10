#ifndef ISL__HTTP_RESPONSE_STREAM_READER__HXX
#define ISL__HTTP_RESPONSE_STREAM_READER__HXX

#include <isl/HttpMessageStreamReader.hxx>

namespace isl
{

class HttpResponseStreamReader : public HttpMessageStreamReader
{
public:
	HttpResponseStreamReader(AbstractIODevice& device) :
		HttpMessageStreamReader(device),
		_version(),
		_statusCode(),
		_reasonPhrase(),
		_maxVersionLength(MaxVersionLength),
		_maxStatusCodeLength(MaxStatusCodeLength),
		_maxReasonPhraseLength(MaxReasonPhraseLength)
	{}

	inline std::string version() const
	{
		return _version;
	}
	inline std::string statusCode() const
	{
		return _statusCode;
	}
	inline std::string reasonPhrase() const
	{
		return _reasonPhrase;
	}
	inline unsigned int maxVersionLength() const
	{
		return _maxVersionLength;
	}
	inline void setMaxVersionLength(unsigned int newValue)
	{
		_maxVersionLength = newValue;
	}
	inline unsigned int maxStatusCodeLength() const
	{
		return _maxStatusCodeLength;
	}
	inline void setMaxStatusCodeLength(unsigned int newValue)
	{
		_maxStatusCodeLength = newValue;
	}
	inline unsigned int maxReasonPhraseLength() const
	{
		return _maxReasonPhraseLength;
	}
	inline void setMaxReasonPhraseLength(unsigned int newValue)
	{
		_maxReasonPhraseLength = newValue;
	}

	virtual void reset()
	{
		HttpMessageStreamReader::reset();
		_version.clear();
		_statusCode.clear();
		_reasonPhrase.clear();
	}
private:
	HttpResponseStreamReader();

	enum PrivateConstants {
		MaxVersionLength = 20,
		MaxStatusCodeLength = 3,
		MaxReasonPhraseLength = 4096,
	};
	
	virtual bool isAllowedInFirstToken(char ch) const
	{
		return isAllowedInVersion(ch);
	}
	virtual void appendToFirstToken(char ch)
	{
		if (_version.length() >= _maxVersionLength) {
			setIsBad(L"Response HTTP-version is too long");
			return;
		}
		_version.append(1, ch);
	}
	virtual bool isAllowedInSecondToken(char ch) const
	{
		return isDigit(ch);
	}
	virtual void appendToSecondToken(char ch)
	{
		if (_statusCode.length() >= _maxStatusCodeLength) {
			setIsBad(L"Response status code is too long");
			return;
		}
		_statusCode.append(1, ch);
	}
	virtual bool isAllowedInThirdToken(char ch) const
	{
		return (!isControl(ch) || isTab(ch));
	}
	virtual void appendToThirdToken(char ch)
	{
		if (_reasonPhrase.length() >= _maxReasonPhraseLength) {
			setIsBad(L"Response reason phrase is too long");
			return;
		}
		_reasonPhrase.append(1, ch);
	}

	std::string _version;
	std::string _statusCode;
	std::string _reasonPhrase;
	unsigned int _maxVersionLength;
	unsigned int _maxStatusCodeLength;
	unsigned int _maxReasonPhraseLength;
};

} // namespace isl

#endif


#ifndef ISL__HTTP_RESPONSE_STREAM_READER__HXX
#define ISL__HTTP_RESPONSE_STREAM_READER__HXX

#include <isl/HttpMessageStreamReader.hxx>

namespace isl
{

//! HTTP-response stream reader
/*!
  TODO Fetch cookies method
*/
class HttpResponseStreamReader : public HttpMessageStreamReader
{
public:
	//! Constructs HTTP-response stream reader
	/*!
	  \param device Reference to the I/O-device to fetch data from
	  \param bufferSize Data reading buffer size
	*/
	HttpResponseStreamReader(AbstractIODevice& device, size_t bufferSize = DefaultBufferSize) :
		HttpMessageStreamReader(device, bufferSize),
		_version(),
		_statusCode(),
		_reasonPhrase(),
		_maxVersionLength(MaxVersionLength),
		_maxStatusCodeLength(MaxStatusCodeLength),
		_maxReasonPhraseLength(MaxReasonPhraseLength)
	{}

	//! Returns HTTP-version of the HTTP-response
	inline std::string version() const
	{
		return _version;
	}
	//! Returns status code of the HTTP-response
	inline std::string statusCode() const
	{
		return _statusCode;
	}
	//! Returns reason phrase of the HTTP-response
	inline std::string reasonPhrase() const
	{
		return _reasonPhrase;
	}
	//! Returns maximum HTTP-method length
	inline size_t maxVersionLength() const
	{
		return _maxVersionLength;
	}
	//! Sets maximum HTTP-method length
	/*!
	  \param newValue New maximum HTTP-method length value
	*/
	inline void setMaxVersionLength(size_t newValue)
	{
		_maxVersionLength = newValue;
	}
	//! Returns maximum status code length
	inline size_t maxStatusCodeLength() const
	{
		return _maxStatusCodeLength;
	}
	//! Sets maximum status code length
	/*!
	  \param newValue New maximum status code length value
	*/
	inline void setMaxStatusCodeLength(size_t newValue)
	{
		_maxStatusCodeLength = newValue;
	}
	//! Returns maximum reason phrase length
	inline size_t maxReasonPhraseLength() const
	{
		return _maxReasonPhraseLength;
	}
	//! Sets maximum reason phrase length
	/*!
	  \param newValue New maximum reason phrase length value
	*/
	inline void setMaxReasonPhraseLength(size_t newValue)
	{
		_maxReasonPhraseLength = newValue;
	}
	//! Resets HTTP-response stream reader to it's initial state
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
		return Http::isAllowedInVersion(ch);
	}
	virtual void appendToFirstToken(char ch)
	{
		if (_version.length() >= _maxVersionLength) {
			setIsBad(Error(SOURCE_LOCATION_ARGS, "Response HTTP-version is too long"));
			return;
		}
		_version.append(1, ch);
	}
	virtual bool isAllowedInSecondToken(char ch) const
	{
		return Char::isDigit(ch);
	}
	virtual void appendToSecondToken(char ch)
	{
		if (_statusCode.length() >= _maxStatusCodeLength) {
			setIsBad(Error(SOURCE_LOCATION_ARGS, "Response status code is too long"));
			return;
		}
		_statusCode.append(1, ch);
	}
	virtual bool isAllowedInThirdToken(char ch) const
	{
		return (!Http::isControl(ch) || Char::isTab(ch));
	}
	virtual void appendToThirdToken(char ch)
	{
		if (_reasonPhrase.length() >= _maxReasonPhraseLength) {
			setIsBad(Error(SOURCE_LOCATION_ARGS, "Response reason phrase is too long"));
			return;
		}
		_reasonPhrase.append(1, ch);
	}

	std::string _version;
	std::string _statusCode;
	std::string _reasonPhrase;
	size_t _maxVersionLength;
	size_t _maxStatusCodeLength;
	size_t _maxReasonPhraseLength;
};

} // namespace isl

#endif


#ifndef ISL__HTTP_RESPONSE_STREAM_READER__HXX
#define ISL__HTTP_RESPONSE_STREAM_READER__HXX

#include <isl/AbstractHttpMessageStreamReader.hxx>

#ifndef ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_VERSION_LENGTH
#define ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_VERSION_LENGTH 20
#endif
#ifndef ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_STATUS_CODE_LENGTH
#define ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_STATUS_CODE_LENGTH 3
#endif
#ifndef ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_REASON_PHRASE_LENGTH
#define ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_REASON_PHRASE_LENGTH 1024
#endif

namespace isl
{

//! HTTP-response stream reader
class HttpResponseStreamReader : public AbstractHttpMessageStreamReader
{
public:
	enum Constants {
		DefaultMaxVersionLength = ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_VERSION_LENGTH,
		DefaultMaxStatusCodeLength = ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_STATUS_CODE_LENGTH,
		DefaultMaxReasonPhraseLength = ISL__HTTP_REQUEST_STREAM_READER_DEFAULT_MAX_REASON_PHRASE_LENGTH
	};
	//! Constructs HTTP-response stream reader
	/*!
	  \param device Reference to the I/O-device to fetch data from
	  \param bufferSize Data reading buffer size
	  \param maxVersionLength Maximum HTTP-method length
	  \param maxStatusCodeLength Maximum URI length
	  \param maxReasonPhraseLength Maximum HTTP-version length
	*/
	HttpResponseStreamReader(AbstractIODevice& device, size_t bufferSize = DefaultBufferSize,
			size_t maxVersionLength = DefaultMaxVersionLength, size_t maxStatusCodeLength = DefaultMaxStatusCodeLength,
			size_t maxReasonPhraseLength = DefaultMaxReasonPhraseLength);
	//! Returns HTTP-version
	inline std::string version() const
	{
		return parser().firstToken();
	}
	//! Returns status code
	inline std::string statusCode() const
	{
		return parser().secondToken();
	}
	//! Returns reason phrase
	inline std::string reasonPhrase() const
	{
		return parser().thirdToken();
	}
protected:
	//! Parser creation factory method
	virtual HttpMessageParser * createParser() const;
private:
	const size_t _maxVersionLength;
	const size_t _maxStatusCodeLength;
	const size_t _maxReasonPhraseLength;
};

} // namespace isl

#endif

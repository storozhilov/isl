#ifndef ISL__HTTP_RESPONSE_PARSER__HXX
#define ISL__HTTP_RESPONSE_PARSER__HXX

#include <isl/HttpMessageParser.hxx>

#ifndef ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_VERSION_LENGTH
#define ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_VERSION_LENGTH 20
#endif
#ifndef ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_STATUS_CODE_LENGTH
#define ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_STATUS_CODE_LENGTH 3
#endif
#ifndef ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_REASON_PHRASE_LENGTH
#define ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_REASON_PHRASE_LENGTH 1024
#endif

namespace isl
{

//! HTTP-response parser
class HttpResponseParser : public HttpMessageParser
{
public:
	enum Constants {
		DefaultMaxVersionLength = ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_VERSION_LENGTH,
		DefaultMaxStatusCodeLength = ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_STATUS_CODE_LENGTH,
		DefaultMaxReasonPhraseLength = ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_REASON_PHRASE_LENGTH
	};
	//! Constructs HTTP-response parser
	/*!
	  \param maxMethodLength Maximum method length
	  \param maxUriLength Maximum URI length
	  \param maxVersionLength Maximum version length
	  \param maxHeaderNameLength Maximum header name length
	  \param maxHeaderValueLength Maximum header value length
	  \param maxHeadersAmount Maximum headers amount
	*/
	HttpResponseParser(size_t maxVersionLength = DefaultMaxVersionLength,
			size_t maxStatusCodeLength = DefaultMaxStatusCodeLength,
			size_t maxReasonPhraseLength = DefaultMaxReasonPhraseLength,
			size_t maxHeaderNameLength = DefaultMaxHeaderNameLength,
			size_t maxHeaderValueLength = DefaultMaxHeaderValueLength,
			size_t maxHeadersAmount = DefaultMaxHeadersAmount) :
		HttpMessageParser(maxVersionLength, maxStatusCodeLength, maxReasonPhraseLength, maxHeaderNameLength,
				maxHeaderValueLength, maxHeadersAmount)
	{};
	//! Returns HTTP-version
	inline std::string version() const
	{
		return firstToken();
	}
	//! Returns status code
	inline std::string statusCode() const
	{
		return secondToken();
	}
	//! Returns reason phrase
	inline std::string reasonPhrase() const
	{
		return thirdToken();
	}
};

} // namespace isl

#endif

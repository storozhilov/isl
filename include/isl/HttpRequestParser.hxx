#ifndef ISL__HTTP_REQUEST_PARSER__HXX
#define ISL__HTTP_REQUEST_PARSER__HXX

#include <isl/HttpMessageParser.hxx>

#ifndef ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_METHOD_LENGTH
#define ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_METHOD_LENGTH 20
#endif
#ifndef ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_URI_LENGTH
#define ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_URI_LENGTH 4096
#endif
#ifndef ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_VERSION_LENGTH
#define ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_VERSION_LENGTH 20
#endif

namespace isl
{

//! HTTP-request parser
class HttpRequestParser : public HttpMessageParser
{
public:
	enum Constants {
		DefaultMaxMethodLength = ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_METHOD_LENGTH,
		DefaultMaxUriLength = ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_URI_LENGTH,
		DefaultMaxVersionLength = ISL__HTTP_REQUEST_PARSER_DEFAULT_MAX_VERSION_LENGTH
	};

	//! Constructs HTTP-request parser
	/*!
	  \param maxMethodLength Maximum method length
	  \param maxUriLength Maximum URI length
	  \param maxVersionLength Maximum version length
	  \param maxHeaderNameLength Maximum header name length
	  \param maxHeaderValueLength Maximum header value length
	  \param maxHeadersAmount Maximum headers amount
	*/
	HttpRequestParser(size_t maxMethodLength = DefaultMaxMethodLength,
			size_t maxUriLength = DefaultMaxUriLength,
			size_t maxVersionLength = DefaultMaxVersionLength,
			size_t maxHeaderNameLength = DefaultMaxHeaderNameLength,
			size_t maxHeaderValueLength = DefaultMaxHeaderValueLength,
			size_t maxHeadersAmount = DefaultMaxHeadersAmount) :
		HttpMessageParser(maxMethodLength, maxUriLength, maxVersionLength, maxHeaderNameLength,
				maxHeaderValueLength, maxHeadersAmount)
	{};
	//! Returns HTTP-method
	inline const std::string& method() const
	{
		return firstToken();
	}
	//! Returns URI
	inline const std::string& uri() const
	{
		return secondToken();
	}
	//! Returns HTTP-version
	inline const std::string& version() const
	{
		return thirdToken();
	}
};

} // namespace isl

#endif

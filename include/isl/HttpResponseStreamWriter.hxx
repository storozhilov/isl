#ifndef ISL__HTTP_RESPONSE_STREAM_WRITER__HXX
#define ISL__HTTP_RESPONSE_STREAM_WRITER__HXX

#include <isl/HttpMessageStreamWriter.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>

namespace isl
{

//! HTTP-response stream writer
/*!
  TODO <tt>setCookie(...)</tt> method
*/
class HttpResponseStreamWriter : public HttpMessageStreamWriter
{
public:
	//! Constructs HTTP-response stream writer
	/*!
	  \param device Reference to the I/O-device to write data to
	  \param statusCode Status code
	  \param reasonPhrase Reason phrase
	  \param version HTTP-version
	*/
	HttpResponseStreamWriter(AbstractIODevice& device, const std::string& statusCode = std::string("200"), const std::string& reasonPhrase = std::string(), const std::string& version = std::string("HTTP/1.1")) :
		HttpMessageStreamWriter(device),
		_version(version),
		_statusCode(statusCode),
		_reasonPhrase(reasonPhrase.empty() ? lookupReasonPhrase(statusCode) : reasonPhrase)
	{}
	//! Returns status code
	inline std::string statusCode() const
	{
		return _statusCode;
	}
	//! Returns reason phrase
	inline std::string reasonPhrase() const
	{
		return _reasonPhrase;
	}
	//! Returns HTTP-version
	inline std::string version() const
	{
		return _version;
	}
	//! Resets HTTP-response stream writer
	/*!
	  \param statusCode Status code
	*/
	inline void reset(const std::string& statusCode)
	{
		HttpMessageStreamWriter::reset();
		_statusCode = statusCode;
		_reasonPhrase = lookupReasonPhrase(statusCode);
	}
	//! Resets HTTP-response stream writer
	/*!
	  \param statusCode Status code
	  \param reasonPhrase Reason phrase
	*/
	inline void reset(const std::string& statusCode, const std::string& reasonPhrase)
	{
		HttpMessageStreamWriter::reset();
		_statusCode = statusCode;
		_reasonPhrase = reasonPhrase;
	}
	//! Resets HTTP-response stream writer
	/*!
	  \param statusCode Status code
	  \param reasonPhrase Reason phrase
	  \param version HTTP-version
	*/
	inline void reset(const std::string& statusCode, const std::string& reasonPhrase, const std::string& version)
	{
		HttpMessageStreamWriter::reset();
		_statusCode = statusCode;
		_reasonPhrase = lookupReasonPhrase(statusCode);
		_version = version;
	}
	//void setCookie(...)
private:
	HttpResponseStreamWriter();

	static std::string lookupReasonPhrase(const std::string& statusCode)
	{
		if (statusCode == "100") {
			return "Continue";
		} else if (statusCode == "101") {
			return "Switching Protocols";
		} else if (statusCode == "200") {
			return "OK";
		} else if (statusCode == "201") {
			return "Created";
		} else if (statusCode == "202") {
			return "Accepted";
		} else if (statusCode == "203") {
			return "Non-Authoritative Information";
		} else if (statusCode == "204") {
			return "No Content";
		} else if (statusCode == "205") {
			return "Reset Content";
		} else if (statusCode == "206") {
			return "Partial Content";
		} else if (statusCode == "300") {
			return "Multiple Choices";
		} else if (statusCode == "301") {
			return "Moved Permanently";
		} else if (statusCode == "302") {
			return "Found";
		} else if (statusCode == "303") {
			return "See Other";
		} else if (statusCode == "304") {
			return "Not Modified";
		} else if (statusCode == "305") {
			return "Use Proxy";
		} else if (statusCode == "307") {
			return "Temporary Redirect";
		} else if (statusCode == "400") {
			return "Bad Request";
		} else if (statusCode == "401") {
			return "Unauthorized";
		} else if (statusCode == "402") {
			return "Payment Required";
		} else if (statusCode == "403") {
			return "Forbidden";
		} else if (statusCode == "404") {
			return "Not Found";
		} else if (statusCode == "405") {
			return "Method Not Allowed";
		} else if (statusCode == "406") {
			return "Not Acceptable";
		} else if (statusCode == "407") {
			return "Proxy Authentication Required";
		} else if (statusCode == "408") {
			return "Request Time-out";
		} else if (statusCode == "409") {
			return "Conflict";
		} else if (statusCode == "410") {
			return "Gone";
		} else if (statusCode == "411") {
			return "Length Required";
		} else if (statusCode == "412") {
			return "Precondition Failed";
		} else if (statusCode == "413") {
			return "Request Entity Too Large";
		} else if (statusCode == "414") {
			return "Request-URI Too Large";
		} else if (statusCode == "415") {
			return "Unsupported Media Type";
		} else if (statusCode == "416") {
			return "Requested range not satisfiable";
		} else if (statusCode == "417") {
			return "Expectation Failed";
		} else if (statusCode == "500") {
			return "Internal Server Error";
		} else if (statusCode == "501") {
			return "Not Implemented";
		} else if (statusCode == "502") {
			return "Bad Gateway";
		} else if (statusCode == "503") {
			return "Service Unavailable";
		} else if (statusCode == "504") {
			return "Gateway Time-out";
		} else if (statusCode == "505") {
			return "HTTP Version not supported";
		} else {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Unknown status code"));
		}
	}

	virtual std::string composeFirstLine() const
	{
		std::string firstLine(_version);
		firstLine.append(1, ' ');
		firstLine.append(_statusCode);
		firstLine.append(1, ' ');
		firstLine.append(_reasonPhrase);
		firstLine.append("\r\n");
		return firstLine;
	}

	std::string _version;
	std::string _statusCode;
	std::string _reasonPhrase;
};

} // namespace isl

#endif


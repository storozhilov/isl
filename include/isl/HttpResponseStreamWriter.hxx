#ifndef ISL__HTTP_RESPONSE_STREAM_WRITER__HXX
#define ISL__HTTP_RESPONSE_STREAM_WRITER__HXX

#include <isl/AbstractHttpMessageStreamWriter.hxx>

namespace isl
{

//! HTTP-response stream writer
/*!
  TODO <tt>setCookie(...)</tt> method
*/
class HttpResponseStreamWriter : public AbstractHttpMessageStreamWriter
{
public:
        static const int defaultStatusCode = 200;
        static const char * defaultVersion;

        //! Constructs HTTP-response stream writer with default parameters
	HttpResponseStreamWriter();
	//! Constructs HTTP-response stream writer with particular status code and default version
	/*!
          A reason phrase is identified by status code.
	  \param statusCode Status code
	*/
	HttpResponseStreamWriter(int statusCode);
	//! Constructs HTTP-response stream writer with particular status code and version.
	/*!
          A reason phrase is identified by status code.
	  \param statusCode Status code
	  \param version HTTP-version
	*/
	HttpResponseStreamWriter(int statusCode, const std::string& version);
	//! Constructs HTTP-response stream writer with particular status code, version and reason phrase
	/*!
	  \param statusCode Status code
	  \param version HTTP-version
	  \param reasonPhrase Reason phrase
	*/
	HttpResponseStreamWriter(int statusCode, const std::string& version, const std::string& reasonPhrase);
	//! Returns status code
	inline int statusCode() const
	{
		return _statusCode;
	}
	//! Returns reason phrase
	inline const std::string& reasonPhrase() const
	{
		return _reasonPhrase;
	}
	//! Returns HTTP-version
	inline const std::string& version() const
	{
		return _version;
	}
	//! Resets HTTP-response stream writer
	/*!
	  \param statusCode Status code
	*/
	inline void reset(int statusCode)
	{
		AbstractHttpMessageStreamWriter::reset();
		_statusCode = statusCode;
		_reasonPhrase = lookupReasonPhrase(statusCode);
	}
	//! Resets HTTP-response stream writer
	/*!
	  \param statusCode Status code
	  \param version HTTP-version
	*/
	inline void reset(int statusCode, const std::string& version)
	{
		AbstractHttpMessageStreamWriter::reset();
		_statusCode = statusCode;
		_version = version;
		_reasonPhrase = lookupReasonPhrase(statusCode);
	}
	//! Resets HTTP-response stream writer
	/*!
	  \param statusCode Status code
	  \param version HTTP-version
	  \param reasonPhrase Reason phrase
	*/
	inline void reset(int statusCode, const std::string& version, const std::string& reasonPhrase)
	{
		AbstractHttpMessageStreamWriter::reset();
		_statusCode = statusCode;
		_version = version;
		_reasonPhrase = reasonPhrase;
	}
	//void setCookie(...)
private:
	struct Status
	{
		int code;
		const char * reasonPhrase;
	};

	static const char * lookupReasonPhrase(int statusCode);

	virtual std::string composeFirstLine() const;

	static Status statusContainer[];

	std::string _version;
	int _statusCode;
	std::string _reasonPhrase;
};

} // namespace isl

#endif

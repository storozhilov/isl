#ifndef ISL__HTTP_MESSAGE_STREAM_WRITER__HXX
#define ISL__HTTP_MESSAGE_STREAM_WRITER__HXX

#include <isl/AbstractIODevice.hxx>
#include <isl/Http.hxx>
#include <string>
#include <map>
#include <list>
#include <string.h>

namespace isl
{

//! Base class for HTTP-message stream writers
// TODO setCookie(...) method
class HttpMessageStreamWriter
{
public:
	//! Constructor
	/*!
	  \param device I/O-device for data to send
	*/
	HttpMessageStreamWriter(AbstractIODevice& device);
	//! Destructor
	virtual ~HttpMessageStreamWriter();
	//! Returns a reference to the I/O device
	inline AbstractIODevice& device() const
	{
		return _device;
	}
	//! Resets writer
	void reset();
	//! Sets header field
	/*!
	  \param fieldName Name of the HTTP-header field.
	  \param fieldValue Value of the HTTP-header field.
	  \param replaceIfExists Replace header value if it exists and has not transmitted to the client.
	*/
	void setHeaderField(const std::string &fieldName, const std::string &fieldValue, bool replaceIfExists = true);
	//! Inspects HTTP-header for specified field name
	/*!
	  \param fieldName Name of the HTTP-header field.
	  \return TRUE if HTTP-header field exists.
	*/
	inline bool headerContains(const std::string &fieldName) const
	{
		std::pair<Header::const_iterator, Header::const_iterator> range = _header.equal_range(fieldName);
		return range.first != range.second;
	}
	//! Inspects HTTP-header for specified field name and value
	/*!
	  \param fieldName Name of the HTTP-header field.
	  \param fieldValue Value of the HTTP-header field.
	  \return TRUE if HTTP-header field exists.
	*/
	bool headerContains(const std::string &fieldName, const std::string &fieldValue) const;
	//! Returns HTTP-header value of the specified field
	/*!
	  \param fieldName Name of the HTTP-header field.
	  \return Value of the HTTP-header field or empty string if it does not exists
	*/
	inline std::string headerValue(const std::string &fieldName) const
	{
		std::pair<Header::const_iterator, Header::const_iterator> range = _header.equal_range(fieldName);
		return range.first == range.second ? std::string() : range.first->second.first;
	}
	//! Returns HTTP-header values list of the specified field
	/*!
	  \param fieldName Name of the HTTP-header field.
	  \return Values list of the HTTP-header field or empty list if it does not exists
	*/
	std::list<std::string> headerValues(const std::string &fieldName) const;
	//! Returns all headers as an associative array
	HttpHeader header() const;
	//! Removes header field
	/*!
	  \param fieldName Name of the HTTP-header field to be removed.
	*/
	void removeHeaderField(const std::string &fieldName);
	//! Returns true if the transmission has been already started on device
	inline bool transmissionStarted() const
	{
		return _transmissionStarted;
	}
	//! Returns true if flush() call is needed
	inline bool needFlush() const
	{
		return !_sendBuffer.empty();
	}
	//! Sends chunked encoded NULL-terminated string
	/*!
	  If string is empty this method does nothing and returns true.
	  \param str NULL-terminated string to send
	  \param timeout I/O timeout
	  \return TRUE if all data has been send to peer or FALSE if the I/O timeout has been expired and flush() call is needed to complete an operation
	*/
	inline bool writeChunk(const char * str, const Timeout& timeout = Timeout())
	{
		return writeChunk(str, strlen(str), timeout);
	}
	//! Sends chunked encoded buffer
	/*!
	  If bufferSize <= 0 this method does nothing and returns true.
	  \param buffer Buffer to send
	  \param bufferSize Buffer size
	  \param timeout I/O timeout
	  \return TRUE if all data has been send to peer or FALSE if the I/O timeout has been expired and flush() call is needed to complete an operation
	*/
	bool writeChunk(const char * buffer, unsigned int bufferSize, const Timeout& timeout = Timeout());
	//! Sends unencoded NULL-terminated string and finalizes HTTP-message
	/*!
	  \param str NULL-terminated string to send
	  \param timeout I/O timeout
	  \return TRUE if all data has been send to peer or FALSE if the I/O timeout has been expired and flush() call is needed to complete an operation
	*/
	inline bool writeUnencoded(const char * str, const Timeout& timeout = Timeout())
	{
		writeUnencoded(str, strlen(str), timeout);
	}
	//! Sends unencoded buffer and finalizes HTTP-message
	/*!
	  \param buffer Buffer to send
	  \param bufferSize Buffer size
	  \param timeout I/O timeout
	  \return TRUE if all data has been send to peer or FALSE if the I/O timeout has been expired and flush() call is needed to complete an operation
	*/
	bool writeUnencoded(const char * buffer, unsigned int bufferSize, const Timeout& timeout = Timeout());
	//! Sends bodyless HTTP-message
	inline bool writeBodyless(const Timeout& timeout = Timeout())
	{
		writeUnencoded(0, 0, timeout);
	}
	//! Sends last chunk and the trailer of the HTTP-message
	/*!
	  NOTE: If no chunks where sent to client before, this call behaves the same as writeBodyless()
	  \param timeout I/O timeout
	  \return TRUE if all data has been send to peer or FALSE if the I/O timeout has been expired and flush() call is needed to complete an operation
	*/
	bool finalize(const Timeout& timeout = Timeout());
	//! Sends all unsent data
	/*!
	  \param timeout I/O timeout
	  \return TRUE if all data has been send to peer or FALSE if the I/O timeout has been expired and flush() call is needed to complete an operation
	*/
	bool flush(const Timeout& timeout = Timeout());
protected:
	virtual std::string composeFirstLine() const = 0;
private:
	HttpMessageStreamWriter();

	typedef std::multimap<std::string, std::pair<std::string, bool> > Header;

	std::string composeHeader();

	AbstractIODevice& _device;
	Header _header;
	bool _transmissionStarted;
	bool _chunkedHeaderComposed;
	bool _isFinalizing;
	std::string _sendBuffer;
	unsigned int _sendBufferBytesSent;
};

} // namespace isl

#endif

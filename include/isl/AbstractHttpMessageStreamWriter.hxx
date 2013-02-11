#ifndef ISL__ABSTRACT_HTTP_MESSAGE_STREAM_WRITER__HXX
#define ISL__ABSTRACT_HTTP_MESSAGE_STREAM_WRITER__HXX

#include <isl/AbstractIODevice.hxx>
#include <isl/Http.hxx>
#include <isl/Timestamp.hxx>
#include <string>
#include <map>
#include <list>
#include <string.h>

namespace isl
{

//! Base abstract class for HTTP-message stream writers
class AbstractHttpMessageStreamWriter
{
public:
	//! Constructor
	AbstractHttpMessageStreamWriter();
	//! Destructor
	virtual ~AbstractHttpMessageStreamWriter();
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
	Http::Params header() const;
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
	//! Sends chunked encoded STL string
	/*!
	  If string is empty this method does nothing and returns true.
	  \param device I/O-device for data to send
	  \param str STL string to send
	  \param limit Data transfer limit timestamp
	  \param bytesWrittenToDevice Pointer to memory location where number of bytes have been sent to the device is to be put
	  \return TRUE if all data has been send to peer or FALSE if the I/O timeout has been expired and flush() call is needed to complete an operation
	*/
	inline bool writeChunk(AbstractIODevice& device, const std::string& str, const Timestamp& limit, size_t * bytesWrittenToDevice = 0)
	{
		return writeChunk(device, str.data(), str.length(), limit, bytesWrittenToDevice);
	}
	//! Sends chunked encoded NULL-terminated string
	/*!
	  If string is empty this method does nothing and returns true.
	  \param device I/O-device for data to send
	  \param str NULL-terminated string to send
	  \param limit Data transfer limit timestamp
	  \param bytesWrittenToDevice Pointer to memory location where number of bytes have been sent to the device is to be put
	  \return TRUE if all data has been send to peer or FALSE if the I/O timeout has been expired and flush() call is needed to complete an operation
	*/
	inline bool writeChunk(AbstractIODevice& device, const char * str, const Timestamp& limit, size_t * bytesWrittenToDevice = 0)
	{
		return writeChunk(device, str, strlen(str), limit, bytesWrittenToDevice);
	}
	//! Sends chunked encoded buffer
	/*!
	  If bufferSize <= 0 this method does nothing and returns true.
	  \param device I/O-device for data to send
	  \param buffer Buffer to send
	  \param bufferSize Buffer size
	  \param limit Data transfer limit timestamp
	  \param bytesWrittenToDevice Pointer to memory location where number of bytes have been sent to the device is to be put
	  \return TRUE if all data has been send to peer or FALSE if the I/O timeout has been expired and flush() call is needed to complete an operation
	*/
	bool writeChunk(AbstractIODevice& device, const char * buffer, size_t bufferSize, const Timestamp& limit, size_t * bytesWrittenToDevice = 0);
	//! Sends unencoded STL string and finalizes HTTP-message
	/*!
	  \param device I/O-device for data to send
	  \param str STL string to send
	  \param limit Data transfer limit timestamp
	  \param bytesWrittenToDevice Pointer to memory location where number of bytes have been sent to the device is to be put
	  \return TRUE if all data has been send to peer or FALSE if the I/O timeout has been expired and flush() call is needed to complete an operation
	*/
	inline bool writeOnce(AbstractIODevice& device, const std::string& str, const Timestamp& limit, size_t * bytesWrittenToDevice = 0)
	{
		return writeOnce(device, str.data(), str.length(), limit, bytesWrittenToDevice);
	}
	//! Sends unencoded NULL-terminated string and finalizes HTTP-message
	/*!
	  \param device I/O-device for data to send
	  \param str NULL-terminated string to send
	  \param limit Data transfer limit timestamp
	  \param bytesWrittenToDevice Pointer to memory location where number of bytes have been sent to the device is to be put
	  \return TRUE if all data has been send to peer or FALSE if the I/O timeout has been expired and flush() call is needed to complete an operation
	*/
	inline bool writeOnce(AbstractIODevice& device, const char * str, const Timestamp& limit, size_t * bytesWrittenToDevice = 0)
	{
		return writeOnce(device, str, strlen(str), limit, bytesWrittenToDevice);
	}
	//! Sends unencoded buffer and finalizes HTTP-message
	/*!
	  \param device I/O-device for data to send
	  \param buffer Buffer to send
	  \param bufferSize Buffer size
	  \param limit Data transfer limit timestamp
	  \param bytesWrittenToDevice Pointer to memory location where number of bytes have been sent to the device is to be put
	  \return TRUE if all data has been send to peer or FALSE if the I/O timeout has been expired and flush() call is needed to complete an operation
	*/
	bool writeOnce(AbstractIODevice& device, const char * buffer, size_t bufferSize, const Timestamp& limit, size_t * bytesWrittenToDevice = 0);
	//! Sends bodyless HTTP-message
	/*!
	  \param device I/O-device for data to send
	  \param limit Data transfer limit timestamp
	  \param bytesWrittenToDevice Pointer to memory location where number of bytes have been sent to the device is to be put
	  \return TRUE if all data has been send to peer or FALSE if the I/O timeout has been expired and flush() call is needed to complete an operation
	*/
	inline bool writeBodyless(AbstractIODevice& device, const Timestamp& limit, size_t * bytesWrittenToDevice = 0)
	{
		return writeOnce(device, 0, 0, limit, bytesWrittenToDevice);
	}
	//! Sends last (empty) chunk and the trailer of the HTTP-message
	/*!
	  \param device I/O-device for data to send
	  \param limit Data transfer limit timestamp
	  \param bytesWrittenToDevice Pointer to memory location where number of bytes have been sent to the device is to be put
	  \return TRUE if all data has been send to peer or FALSE if the I/O timeout has been expired and flush() call is needed to complete an operation
	  \note If no chunks where sent to client before, this call behaves the same as writeBodyless()
	*/
	bool finalize(AbstractIODevice& device, const Timestamp& limit, size_t * bytesWrittenToDevice = 0);
	//! Sends all unsent data
	/*!
	  \param device I/O-device for data to send
	  \param limit Data transfer limit timestamp
	  \param bytesWrittenToDevice Pointer to memory location where number of bytes have been sent to the device is to be put
	  \return TRUE if all data has been send to peer or FALSE if the I/O timeout has been expired and flush() call is needed to complete an operation
	*/
	bool flush(AbstractIODevice& device, const Timestamp& limit, size_t * bytesWrittenToDevice = 0);
	//! Resets writer to it's initial state
	void reset();
protected:
	//! HTTP-message first line composition method
	virtual std::string composeFirstLine() const = 0;
private:
	typedef std::multimap<std::string, std::pair<std::string, bool> > Header;

	std::string composeHeader();
	bool flushBuffer(AbstractIODevice& device, const Timestamp& limit, size_t * bytesWrittenToDevice = 0);

	Header _header;
	bool _transmissionStarted;
	bool _chunkedHeaderComposed;
	bool _isFinalizing;
	std::string _sendBuffer;
	size_t _bytesSent;
};

} // namespace isl

#endif

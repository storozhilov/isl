#include <isl/AbstractHttpMessageStreamWriter.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <sstream>

namespace isl
{

AbstractHttpMessageStreamWriter::AbstractHttpMessageStreamWriter() :
	_header(),
	_transmissionStarted(false),
	_chunkedHeaderComposed(false),
	_isFinalizing(false),
	_sendBuffer(),
	_bytesSent(0)
{}

AbstractHttpMessageStreamWriter::~AbstractHttpMessageStreamWriter()
{}

void AbstractHttpMessageStreamWriter::setHeaderField(const std::string &fieldName, const std::string &fieldValue, bool replaceIfExists)
{
	if (replaceIfExists) {
		std::pair<Header::iterator, Header::iterator> range = _header.equal_range(fieldName);
		for (Header::iterator i = range.first; i != range.second; ++i) {
			if (i->second.second) {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "Header field to replace has been already composed for sending"));
			}
			_header.erase(i);
		}
	}
	_header.insert(Header::value_type(fieldName, std::pair<std::string, bool>(fieldValue, false)));
}

bool AbstractHttpMessageStreamWriter::headerContains(const std::string &fieldName, const std::string &fieldValue) const
{
	std::pair<Header::const_iterator, Header::const_iterator> range = _header.equal_range(fieldName);
	for (Header::const_iterator i = range.first; i != range.second; ++i) {
		if (i->second.first == fieldValue) {
			return true;
		}
	}
	return false;
}

std::list<std::string> AbstractHttpMessageStreamWriter::headerValues(const std::string &fieldName) const
{
	std::list<std::string> result;
	std::pair<Header::const_iterator, Header::const_iterator> range = _header.equal_range(fieldName);
	for (Header::const_iterator i = range.first; i != range.second; ++i) {
		result.push_back(i->second.first);
	}
	return result;
}

Http::Params AbstractHttpMessageStreamWriter::header() const
{
	Http::Params result;
	for (Header::const_iterator i = _header.begin(); i != _header.end(); ++i) {
		result.insert(Http::Params::value_type(i->first, i->second.first));
	}
	return result;
}

void AbstractHttpMessageStreamWriter::removeHeaderField(const std::string &fieldName)
{
	std::pair<Header::iterator, Header::iterator> range = _header.equal_range(fieldName);
	for (Header::iterator i = range.first; i != range.second; ++i) {
		if (i->second.second) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Header field to remove has been already composed for sending"));
		}
	}
	_header.erase(range.first, range.second);
}

bool AbstractHttpMessageStreamWriter::writeChunk(AbstractIODevice& device, const char * buffer, size_t bufferSize, const Timestamp& limit, size_t * bytesWrittenToDevice)
{
	if (bytesWrittenToDevice) {
		*bytesWrittenToDevice = 0;
	}
	if (needFlush()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Could not send data - flush needed"));
	}
	if (bufferSize <= 0) {
		return true;
	}
	// Composing data to send
	if (!_chunkedHeaderComposed) {
		// Composing header if not comosed one
		setHeaderField("Transfer-Encoding", "chunked");
		_sendBuffer.append(composeFirstLine());
		_sendBuffer.append(composeHeader());
		_sendBuffer.append("\r\n");
		_chunkedHeaderComposed = true;
	}
	// Composing chunk
	std::ostringstream chunkSize;
	chunkSize << std::hex << bufferSize;
	_sendBuffer.append(chunkSize.str());
	_sendBuffer.append("\r\n");
	_sendBuffer.append(buffer, bufferSize);
	_sendBuffer.append("\r\n");
	// Sending the data
	_bytesSent = 0;
	return flushBuffer(device, limit, bytesWrittenToDevice);
}

bool AbstractHttpMessageStreamWriter::writeOnce(AbstractIODevice& device, const char * buffer, size_t bufferSize, const Timestamp& limit, size_t * bytesWrittenToDevice)
{
	if (bytesWrittenToDevice) {
		*bytesWrittenToDevice = 0;
	}
	if (_chunkedHeaderComposed) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Could not send unencoded data while chunked encoding"));
	}
	if (needFlush()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Could not send data cause flush is needed"));
	}
	removeHeaderField("Transfer-Encoding");
	if (bufferSize > 0) {
		std::ostringstream contentLength;
		contentLength << bufferSize;
		setHeaderField("Content-Length", contentLength.str());
	} else {
		removeHeaderField("Content-Length");
	}
	// Composing header
	_sendBuffer.append(composeFirstLine());
	_sendBuffer.append(composeHeader());
	_sendBuffer.append("\r\n");
	if (bufferSize > 0) {
		// Composing body
		_sendBuffer.append(buffer, bufferSize);
	}
	// Sending the data
	_bytesSent = 0;
	if (flushBuffer(device, limit, bytesWrittenToDevice)) {
		reset();
		return true;
	} else {
		_isFinalizing = true;
		return false;
	}
}

bool AbstractHttpMessageStreamWriter::finalize(AbstractIODevice& device, const Timestamp& limit, size_t * bytesWrittenToDevice)
{
	if (bytesWrittenToDevice) {
		*bytesWrittenToDevice = 0;
	}
	if (needFlush()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Could not finalize HTTP-request - flush needed"));
	}
	// Composing data to send
	if (_chunkedHeaderComposed) {
		// Composing the last chunk, trailer and final CRLF
		_sendBuffer.append("0\r\n");
		_sendBuffer.append(composeHeader());
		_sendBuffer.append("\r\n");
	} else {
		// Composing header only
		removeHeaderField("Content-Length");
		removeHeaderField("Transfer-Encoding");
		_sendBuffer.append(composeFirstLine());
		_sendBuffer.append(composeHeader());
		_sendBuffer.append("\r\n");
	}
	// Sending the data
	_bytesSent = 0;
	if (flushBuffer(device, limit, bytesWrittenToDevice)) {
		reset();
		return true;
	} else {
		_isFinalizing = true;
		return false;
	}
}

bool AbstractHttpMessageStreamWriter::flush(AbstractIODevice& device, const Timestamp& limit, size_t * bytesWrittenToDevice)
{
	if (bytesWrittenToDevice) {
		*bytesWrittenToDevice = 0;
	}
	if (!needFlush()) {
		return true;
	}
	bool sendBufferFlushed = flushBuffer(device, limit, bytesWrittenToDevice);
	if (sendBufferFlushed && _isFinalizing) {
		reset();
	}
	return sendBufferFlushed;
}

void AbstractHttpMessageStreamWriter::reset()
{
	_header.clear();
	_transmissionStarted = false;
	_chunkedHeaderComposed = false;
	_isFinalizing = false;
	_sendBuffer.clear();
	_bytesSent = 0;
}

std::string AbstractHttpMessageStreamWriter::composeHeader()
{
	std::string result;
	for (Header::iterator i = _header.begin(); i != _header.end(); ++i) {
		if (i->second.second) {
			continue;
		}
		result += i->first;
		result += ": ";
		result += i->second.first;
		result += "\r\n";
		i->second.second = true;
	}
	return result;
}

bool AbstractHttpMessageStreamWriter::flushBuffer(AbstractIODevice& device, const Timestamp& limit, size_t * bytesWrittenToDevice)
{
	if (bytesWrittenToDevice) {
		*bytesWrittenToDevice = 0;
	}
	while (true) {
		size_t bytesSent = device.write(_sendBuffer.data() + _bytesSent, _sendBuffer.size() - _bytesSent, limit.leftTo());
		if (bytesSent > 0) {
			// Some data has been sent
			if (!_transmissionStarted) {
				_transmissionStarted = true;
			}
			_bytesSent += bytesSent;
			if (bytesWrittenToDevice) {
				(*bytesWrittenToDevice) += bytesSent;
			}
			if (_bytesSent >= _sendBuffer.size()) {
				// Buffer has been flushed -> reset buffer
				_sendBuffer.clear();
				_bytesSent = 0;
				return true;
			}
			if (Timestamp::now() >= limit) {
				return false;
			}
		} else {
			// No data has been sent -> timeout expired
			return false;
		}
	}
}

} // namespace isl

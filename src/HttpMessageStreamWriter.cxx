#include <isl/HttpMessageStreamWriter.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <sstream>

namespace isl
{

HttpMessageStreamWriter::HttpMessageStreamWriter(AbstractIODevice& device) :
	_device(device),
	_header(),
	_transmissionStarted(false),
	_chunkedHeaderComposed(false),
	_isFinalizing(false),
	_sendBuffer(),
	_sendBufferBytesSent(0)
{}

HttpMessageStreamWriter::~HttpMessageStreamWriter()
{}

void HttpMessageStreamWriter::reset()
{
	_header.clear();
	_transmissionStarted = false;
	_chunkedHeaderComposed = false;
	_isFinalizing = false;
	_sendBuffer.clear();
	_sendBufferBytesSent = 0;
}

void HttpMessageStreamWriter::setHeaderField(const std::string &fieldName, const std::string &fieldValue, bool replaceIfExists)
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

bool HttpMessageStreamWriter::headerContains(const std::string &fieldName, const std::string &fieldValue) const
{
	std::pair<Header::const_iterator, Header::const_iterator> range = _header.equal_range(fieldName);
	for (Header::const_iterator i = range.first; i != range.second; ++i) {
		if (i->second.first == fieldValue) {
			return true;
		}
	}
	return false;
}

std::list<std::string> HttpMessageStreamWriter::headerValues(const std::string &fieldName) const
{
	std::list<std::string> result;
	std::pair<Header::const_iterator, Header::const_iterator> range = _header.equal_range(fieldName);
	for (Header::const_iterator i = range.first; i != range.second; ++i) {
		result.push_back(i->second.first);
	}
	return result;
}

Http::Params HttpMessageStreamWriter::header() const
{
	Http::Params result;
	for (Header::const_iterator i = _header.begin(); i != _header.end(); ++i) {
		result.insert(Http::Params::value_type(i->first, i->second.first));
	}
	return result;
}

void HttpMessageStreamWriter::removeHeaderField(const std::string &fieldName)
{
	std::pair<Header::iterator, Header::iterator> range = _header.equal_range(fieldName);
	for (Header::iterator i = range.first; i != range.second; ++i) {
		if (i->second.second) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Header field to remove has been already composed for sending"));
		}
	}
	_header.erase(range.first, range.second);
}

bool HttpMessageStreamWriter::writeChunk(const char * buffer, size_t bufferSize, const Timeout& timeout, size_t * bytesWrittenToDevice)
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
	if (bufferSize > 0) {
		// Composing chunk
		std::ostringstream chunkSize;
		chunkSize << std::hex << bufferSize;
		_sendBuffer.append(chunkSize.str());
		_sendBuffer.append("\r\n");
		_sendBuffer.append(buffer, bufferSize);
		_sendBuffer.append("\r\n");
	}
	// Sending the data
	_sendBufferBytesSent = _device.write(_sendBuffer.data(), _sendBuffer.size(), timeout);
	if (!_transmissionStarted && (_sendBufferBytesSent > 0)) {
		_transmissionStarted = true;
	}
	if (bytesWrittenToDevice) {
		*bytesWrittenToDevice = _sendBufferBytesSent;
	}
	if (_sendBufferBytesSent >= _sendBuffer.size()) {
		_sendBuffer.clear();
		_sendBufferBytesSent = 0;
		return true;
	} else {
		return false;
	}
}

bool HttpMessageStreamWriter::writeOnce(const char * buffer, size_t bufferSize, const Timeout& timeout, size_t * bytesWrittenToDevice)
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
	_sendBufferBytesSent = _device.write(_sendBuffer.data(), _sendBuffer.size(), timeout);
	if (!_transmissionStarted && (_sendBufferBytesSent > 0)) {
		_transmissionStarted = true;
	}
	if (bytesWrittenToDevice) {
		*bytesWrittenToDevice = _sendBufferBytesSent;
	}
	if (_sendBufferBytesSent >= _sendBuffer.size()) {
		_sendBuffer.clear();
		_sendBufferBytesSent = 0;
		return true;
	} else {
		_isFinalizing = true;
		return false;
	}
}

bool HttpMessageStreamWriter::finalize(const Timeout& timeout, size_t * bytesWrittenToDevice)
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
	_sendBufferBytesSent = _device.write(_sendBuffer.data(), _sendBuffer.size(), timeout);
	if (!_transmissionStarted && (_sendBufferBytesSent > 0)) {
		_transmissionStarted = true;
	}
	if (bytesWrittenToDevice) {
		*bytesWrittenToDevice = _sendBufferBytesSent;
	}
	if (_sendBufferBytesSent >= _sendBuffer.size()) {
		reset();
		return true;
	} else {
		_isFinalizing = true;
		return false;
	}
	reset();
}

bool HttpMessageStreamWriter::flush(const Timeout& timeout, size_t * bytesWrittenToDevice)
{
	if (bytesWrittenToDevice) {
		*bytesWrittenToDevice = 0;
	}
	if (!needFlush()) {
		return true;
	}
	size_t bytesSent = _device.write(_sendBuffer.data() + _sendBufferBytesSent, _sendBuffer.size() - _sendBufferBytesSent, timeout);
	if (!_transmissionStarted && (bytesSent > 0)) {
		_transmissionStarted = true;
	}
	if (bytesWrittenToDevice) {
		*bytesWrittenToDevice = bytesSent;
	}
	_sendBufferBytesSent += bytesSent;
	if (_sendBufferBytesSent >= _sendBuffer.size()) {
		if (_isFinalizing) {
			reset();
		} else {
			_sendBuffer.clear();
			_sendBufferBytesSent = 0;
		}
		return true;
	} else {
		return false;
	}
}

std::string HttpMessageStreamWriter::composeHeader()
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

} // namespace isl

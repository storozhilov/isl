#include <isl/HTTPResponse.hxx>
#include <isl/AbstractHTTPTask.hxx>
#include <isl/DateTime.hxx>
#include <isl/String.hxx>
#include <isl/Utf8TextCodec.hxx>
#include <isl/UUID.hxx>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <stdexcept>

namespace isl
{

/*------------------------------------------------------------------------------
 * HTTPResponse
------------------------------------------------------------------------------*/

const char * const HTTPResponse::DefaultServerSignature = "LibISL/0.0.1";

HTTPResponse::HTTPResponse(AbstractHTTPTask * task) :
	HTTPMessage(task),
	_extraHeader(),
	_generators(),
	_sourceBodyBuffer(),
	_destBodyBuffer(*this, DefaultBodyChunkSize),						// TODO Use configuration subsystem
	_statusCode(),
	_cookies(),
	_serverSignature(DefaultServerSignature)						// TODO Use configuration subsystem
{}

HTTPResponse::~HTTPResponse()
{
	resetGenerators();
}

void HTTPResponse::reset()
{
	_extraHeader.clear();
	resetGenerators();
	_destBodyBuffer.reset();
	_statusCode.reset();
	_cookies.clear();
}

HTTPResponse::AbstractBodyBuffer& HTTPResponse::inputBuffer()
{
	return _sourceBodyBuffer;
}

HTTPResponse::AbstractBodyBuffer& HTTPResponse::outputBuffer()
{
	return _destBodyBuffer;
}

HTTPResponse::StatusCode HTTPResponse::statusCode() const
{
	return _statusCode;
}

void HTTPResponse::setStatusCode(HTTPResponse::StatusCode newStatusCode)
{
	_statusCode = newStatusCode;
}

void HTTPResponse::generateAndSend()
{
	reset();
	GeneratorsResetter generatorsResetter(*this);					// Cleanup generators after exit from the method
	try {
		if (_task->_request.isBad()) {
			_version = AbstractParser::Token::construct<HTTP_1_1_Version>();
			_generators.push_back(_task->createGeneratorBadRequest());
		} else {
			_version = _task->_request.version();
			_generators.push_back(_task->createGeneratorOK());
		}
		// TODO Multiple generators
		_generators.front()->generate();
		_destBodyBuffer.flush();
	} catch (...) {
		if (!_destBodyBuffer.transferStarted()) {
			reset();
			_version = AbstractParser::Token::construct<HTTP_1_1_Version>();
			_generators.push_back(_task->createGeneratorInternalServerError());
			_generators.front()->generate();
			_destBodyBuffer.flush();
		} else {
			throw;
		}
	}
}

void HTTPResponse::resetGenerators()
{
	for (GeneratorList::iterator i = _generators.begin(); i != _generators.end(); ++i) {
		delete (*i);
	}
	_generators.clear();
}

/*------------------------------------------------------------------------------
 * HTTPResponse::BodyChunk
------------------------------------------------------------------------------*/

HTTPResponse::BodyChunk::BodyChunk(unsigned int capacity) :
	_capacity(capacity),
	_size(0),
	_buffer(),
	_chunkSizeFieldSize(0),
	_chunckedDataStartPos(0),
	_attemptedToOverflow(false)
{
	// Allocating memory:
	std::ostringstream maxChunkSizeStr;
	maxChunkSizeStr << std::hex << _capacity;
	_chunkSizeFieldSize = maxChunkSizeStr.str().length();
	_buffer.resize(_capacity + _chunkSizeFieldSize + 4);
	// Putting [chunk size/chunk data] CRLF delimeter:
	_buffer[_chunkSizeFieldSize] = '\r';
	_buffer[_chunkSizeFieldSize + 1] = '\n';
	// Initializing:
	setSize(0);
}

HTTPResponse::BodyChunk::~BodyChunk()
{}

void HTTPResponse::BodyChunk::clear()
{
	setSize(0);
	_attemptedToOverflow = false;
}

unsigned int HTTPResponse::BodyChunk::fill(const char * data, unsigned int dataSize)
{
	clear();
	return append(data, dataSize);
}

unsigned int HTTPResponse::BodyChunk::append(const char *data, unsigned int dataSize)
{
	if (dataSize == 0) {
		return 0;
	}
	if (isFull()) {
		_attemptedToOverflow = true;
		return 0;
	}
	unsigned int bytesToCopy = dataSize;
	if ((bytesToCopy + _size) > _capacity) {
		bytesToCopy = _capacity - _size;
		_attemptedToOverflow = true;
	}
	for (unsigned int i = 0; i < bytesToCopy; ++i) {
		_buffer[_chunkSizeFieldSize + 2 + _size + i] = data[i];
	}
	setSize(_size + bytesToCopy);
	return bytesToCopy;
}

bool HTTPResponse::BodyChunk::isFull() const
{
	return (_size >= _capacity);
}

bool HTTPResponse::BodyChunk::BodyChunk::isEmpty() const
{
	return (_size <= 0);
}

bool HTTPResponse::BodyChunk::attemptedToOverflow() const
{
	return _attemptedToOverflow;
}

const char * HTTPResponse::BodyChunk::data() const
{
	return &_buffer[_chunkSizeFieldSize + 2];
}

unsigned int HTTPResponse::BodyChunk::size() const
{
	return _size;
}

const char * HTTPResponse::BodyChunk::chunkEncodedData() const
{
	return &_buffer[_chunckedDataStartPos];
}

unsigned int HTTPResponse::BodyChunk::chunkEncodedSize() const
{
	return _size + _chunkSizeFieldSize - _chunckedDataStartPos + 4;
}

const char * HTTPResponse::BodyChunk::identityEncodedData() const
{
	return data();
}

unsigned int HTTPResponse::BodyChunk::identityEncodedSize() const
{
	return size();
}

void HTTPResponse::BodyChunk::setSize(unsigned int newSize)
{
	_size = newSize;
	// Calculating chunked data start position:
	std::stringstream chunkSizeStr;
	chunkSizeStr << std::hex << _size;
	_chunckedDataStartPos = _chunkSizeFieldSize - chunkSizeStr.str().length();
	// Putting spaces before HEX-ed chunk size to right align it:
	for (unsigned int i = 0; i < _chunckedDataStartPos; ++i) {
		_buffer[i] = 32;
	}
	// Putting chunk size:
	for (unsigned int i = 0; i < chunkSizeStr.str().length(); ++i) {
		_buffer[_chunckedDataStartPos + i] = chunkSizeStr.str()[i];
	}
	// Adding trailing CRLF:
	_buffer[_chunkSizeFieldSize + 2 + _size] = '\r';
	_buffer[_chunkSizeFieldSize + 2 + _size + 1] = '\n';
}

/*------------------------------------------------------------------------------
 * HTTPResponse::AbstractGenerator
------------------------------------------------------------------------------*/

HTTPResponse::AbstractGenerator::AbstractGenerator(AbstractHTTPTask * task) :
	_task(task)
{}

HTTPResponse::AbstractGenerator::~AbstractGenerator()
{}

void HTTPResponse::AbstractGenerator::generate()
{
	response().setStatusCode(statusCode());
	generateImplementation();
}

const HTTPRequest& HTTPResponse::AbstractGenerator::request() const
{
	return _task->_request;
}

HTTPResponse& HTTPResponse::AbstractGenerator::response() const
{
	return _task->_response;
}

AbstractHTTPTask * HTTPResponse::AbstractGenerator::task() const
{
	return _task;
}

/*------------------------------------------------------------------------------
 * HTTPResponse::AbstractGeneratorOK
------------------------------------------------------------------------------*/

HTTPResponse::AbstractGeneratorOK::AbstractGeneratorOK(AbstractHTTPTask * task) :
	HTTPResponse::AbstractGenerator(task)
{}

HTTPResponse::StatusCode HTTPResponse::AbstractGeneratorOK::statusCode() const
{
	return HTTPResponse::StatusCode::construct<HTTPResponse::OKStatusCode>();
}

/*------------------------------------------------------------------------------
 * HTTPResponse::GeneratorBadRequest
------------------------------------------------------------------------------*/

HTTPResponse::GeneratorBadRequest::GeneratorBadRequest(AbstractHTTPTask * task) :
	HTTPResponse::AbstractGenerator(task)
{}

HTTPResponse::StatusCode HTTPResponse::GeneratorBadRequest::statusCode() const
{
	return HTTPResponse::StatusCode::construct<HTTPResponse::BadRequestStatusCode>();
}

void HTTPResponse::GeneratorBadRequest::generateImplementation()
{
	response().outputBuffer().write(
		"<html>\n"
		"  <head>\n"
		"    <title>Bad request</title>\n"
		"  </head>\n"
		"  <body>\n"
		"    <h1>Bad request</h1>\n"
		"    <p>Your browser send request server could not understand.</p>\n"
		"  </body>\n"
		"</html>");
}

/*------------------------------------------------------------------------------
 * HTTPResponse::GeneratorInternalServerError
------------------------------------------------------------------------------*/

HTTPResponse::GeneratorInternalServerError::GeneratorInternalServerError(AbstractHTTPTask * task) :
	HTTPResponse::AbstractGenerator(task)
{}

HTTPResponse::StatusCode HTTPResponse::GeneratorInternalServerError::statusCode() const
{
	return HTTPResponse::StatusCode::construct<HTTPResponse::InternalServerErrorStatusCode>();
}

void HTTPResponse::GeneratorInternalServerError::generateImplementation()
{
	response().outputBuffer().write(
		"<html>\n"
		"  <head>\n"
		"    <title>Internal Server Error</title>\n"
		"  </head>\n"
		"  <body>\n"
		"    <h1>Internal Server Error</h1>\n"
		"    <p>Internal server error occured while serving you.</p>\n"
		"  </body>\n"
		"</html>");
}

/*------------------------------------------------------------------------------
 * HTTPResponse::AbstractBodyBuffer
------------------------------------------------------------------------------*/

HTTPResponse::AbstractBodyBuffer::AbstractBodyBuffer() :
	_inReadingState(false),
	_fileName()
{}

HTTPResponse::AbstractBodyBuffer::~AbstractBodyBuffer()
{}

unsigned int HTTPResponse::AbstractBodyBuffer::read(char * buffer, unsigned int bufferSize)
{
	if (isInFile()) {
		// TODO
		throw std::runtime_error("Body buffer is set to file");
	}
	unsigned int bytesRead = readImplementation(buffer, bufferSize);
	if (!_inReadingState) {
		_inReadingState = true;
	}
	return bytesRead;
}

void HTTPResponse::AbstractBodyBuffer::write(const char * data, unsigned int dataSize)
{
	if (_inReadingState) {
		// TODO
		throw std::runtime_error("Body buffer is in reading state");
	}
	if (isInFile()) {
		// TODO
		throw std::runtime_error("Body buffer is set to file");
	}
	writeImplementation(data, dataSize);
}

void HTTPResponse::AbstractBodyBuffer::write(const char * str)
{
	write(str, strlen(str));
}

void HTTPResponse::AbstractBodyBuffer::write(const std::string& str)
{
	write(str.data(), str.size());
}

void HTTPResponse::AbstractBodyBuffer::clear()
{
	if (_inReadingState) {
		// TODO
		throw std::runtime_error("Body buffer is in reading state");
	}
	if (isInFile()) {
		// TODO
		throw std::runtime_error("Body buffer is set to file");
	}
	clearImplementation();
}

void HTTPResponse::AbstractBodyBuffer::setFile(const std::string& newFileName)
{
	if (_inReadingState) {
		// TODO
		throw std::runtime_error("Body buffer is in reading state");
	}
	// TODO Check if file exists
	if (!isInFile()) {
		clearImplementation();
	}
	_fileName = newFileName;
}

void HTTPResponse::AbstractBodyBuffer::resetFile()
{
	if (_inReadingState) {
		// TODO
		throw std::runtime_error("Body buffer is in reading state");
	}
	_fileName.clear();
}

bool HTTPResponse::AbstractBodyBuffer::isInFile() const
{
	return !_fileName.empty();
}

bool HTTPResponse::AbstractBodyBuffer::isInReadingState() const
{
	return _inReadingState;
}

/*------------------------------------------------------------------------------
 * HTTPResponse::NetworkBodyBuffer
------------------------------------------------------------------------------*/

HTTPResponse::NetworkBodyBuffer::NetworkBodyBuffer(HTTPResponse& response, unsigned int bodyChunkSize) :
	AbstractBodyBuffer(),
	_response(response),
	_bodyChunk(bodyChunkSize),
	_transferStarted(false),
	_isChunkedTransferEncoding(false)
{}

void HTTPResponse::NetworkBodyBuffer::flush()
{
	// Flushing the rest of the data
	if (!_bodyChunk.isEmpty()) {
		sendChunk(true);
		_bodyChunk.clear();
	}
	// Sending last chunk with extra header if chunked transfer encoding
	if (_isChunkedTransferEncoding) {
		std::string buffer("0\r\n");
		// TODO Filter extra header
		if (!_response._extraHeader.empty()) {
			for (HTTPMessage::Header::const_iterator i = _response._extraHeader.begin(); i != _response._extraHeader.end(); ++i) {
				buffer += ((*i).first + ": " + (*i).second + "\r\n");
			}
			buffer += "\r\n";
		}
		buffer += "\r\n";
		std::wostringstream msg;
		msg << L"Sending " << buffer.length() << L" bytes of the last of chunk encoded HTTP-response body";
		AbstractHTTPTask::debugLog.log(msg.str());
		sendBuffer(buffer.data(), buffer.length());
		msg.str(L"");
		msg << buffer.length() << L" of " << buffer.length() << L" bytes of of the last of chunk encoded HTTP-response body sent successfully";
		AbstractHTTPTask::debugLog.log(msg.str());
	}
}

void HTTPResponse::NetworkBodyBuffer::reset()
{
	resetFile();
	_bodyChunk.clear();
	_transferStarted = false;
	_isChunkedTransferEncoding = false;
}

bool HTTPResponse::NetworkBodyBuffer::transferStarted() const
{
	return _transferStarted;
}

void HTTPResponse::NetworkBodyBuffer::sendBuffer(const char * buffer, unsigned int size)
{
	unsigned int bytesSent = _response._task->socket().write(buffer, size, _response._transferTimeout);
	if (bytesSent < size) {
		// TODO
		throw std::runtime_error("Sending HTTP-response header data timeout");
	}
}

void HTTPResponse::NetworkBodyBuffer::sendChunk(bool isLastChunk)
{
	bool transferStartedInitially = _transferStarted;
	if (!_transferStarted) {
		// Composing and sending header
		Header responseHeader(_response._header);
		// Erasing common headers
		responseHeader.erase("Date");
		responseHeader.erase("Server");
		responseHeader.erase("Connection");
		responseHeader.erase("Content-Length");
		responseHeader.erase("Set-Cookie");
		responseHeader.erase("Transfer-Encoding");
		// Setting common headers
		// TODO Use AsciiTextCodec
		responseHeader.insert(Header::value_type("Date", Utf8TextCodec().encode(DateTime::now().toGMT())));
		responseHeader.insert(Header::value_type("Server", _response._serverSignature));
		if (_response._task->connectionToBeClosed()) {
			responseHeader.insert(Header::value_type("Connection", "close"));
		}
		// Determining if the Transfer-Encoding has to be chunked
		if (isLastChunk) {
			std::ostringstream contentLength;
			contentLength << _bodyChunk.identityEncodedSize();
			responseHeader.insert(Header::value_type("Content-Length", contentLength.str()));
		} else {
			responseHeader.insert(Header::value_type("Transfer-Encoding", "chunked"));
			_isChunkedTransferEncoding = true;
		}
		// Adding cookies
		for (HTTPResponse::Cookies::const_iterator i = _response._cookies.begin(); i != _response._cookies.end(); ++i) {
			std::string cookieHeaderFieldValue((*i).name + "=" + String::urlEncode((*i).value));
			if ((*i).expires.isValid()) {
				// TODO Use AsciiTextCodec
				cookieHeaderFieldValue += ("; expires=" + Utf8TextCodec().encode((*i).expires.toGMT()));
			}
			if (!(*i).path.empty()) {
				cookieHeaderFieldValue += ("; path=" + String::urlEncode((*i).path));
			}
			if (!(*i).domain.empty()) {
				cookieHeaderFieldValue += ("; domain=" + String::urlEncode((*i).domain));
			}
			if ((*i).secure) {
				cookieHeaderFieldValue += "; secure";
			}
			responseHeader.insert(Header::value_type("Set-Cookie", cookieHeaderFieldValue));
		}
		// Composing header
		std::string buffer(_response._version.constValue().asString() + ' ' + _response._statusCode.constValue().codeStr() + ' ' +
				_response._statusCode.constValue().reason() + "\r\n");
		for (HTTPMessage::Header::const_iterator i = responseHeader.begin(); i != responseHeader.end(); ++i) {
			buffer += ((*i).first + ": " + (*i).second + "\r\n");
		}
		buffer += "\r\n";
		std::wostringstream msg;
		msg << L"Sending " << buffer.length() << L" bytes of HTTP-response header";
		AbstractHTTPTask::debugLog.log(msg.str());
		sendBuffer(buffer.data(), buffer.length());
		msg.str(L"");
		msg << buffer.length() << L" of " << buffer.length() << L" bytes of HTTP-response header sent successfully";
		AbstractHTTPTask::debugLog.log(msg.str());
		_transferStarted = true;
	}
	// Sending the chunk
	if (_isChunkedTransferEncoding) {
		if (!_transferStarted) {
			// TODO
			throw std::runtime_error("Transfer not started, while sending chunked encoded HTTP-response body");
		}
		std::wostringstream msg;
		msg << L"Sending " << _bodyChunk.chunkEncodedSize() << L" bytes of chunk encoded HTTP-response body";
		AbstractHTTPTask::debugLog.log(msg.str());
		sendBuffer(_bodyChunk.chunkEncodedData(), _bodyChunk.chunkEncodedSize());
		msg.str(L"");
		msg << _bodyChunk.chunkEncodedSize() << L" of " << _bodyChunk.chunkEncodedSize() <<
			L" bytes of chunk encoded HTTP-response body sent successfully";
		AbstractHTTPTask::debugLog.log(msg.str());
	} else if (!_bodyChunk.isEmpty()) {
		// Sending body buffer with identity transfer encoding
		if (transferStartedInitially) {
			// TODO
			throw std::runtime_error("Transfer was initially started while sending indentity encoded HTTP-response");
		} else if (!_transferStarted) {
			// TODO
			throw std::runtime_error("Transfer not started while sending indentity encoded HTTP-response");
		}
		std::wostringstream msg;
		msg << L"Sending " << _bodyChunk.identityEncodedSize() << L" bytes of identity encoded HTTP-response body";
		AbstractHTTPTask::debugLog.log(msg.str());
		sendBuffer(_bodyChunk.identityEncodedData(), _bodyChunk.identityEncodedSize());
		msg.str(L"");
		msg << _bodyChunk.identityEncodedSize() << L" of " << _bodyChunk.identityEncodedSize() <<
			L" bytes of identity encoded HTTP-response body sent successfully";
		AbstractHTTPTask::debugLog.log(msg.str());
	}
}

unsigned int HTTPResponse::NetworkBodyBuffer::readImplementation(char * buffer, unsigned int bufferSize)
{
	// TODO
	throw std::logic_error("Can not read from network body buffer");
}

void HTTPResponse::NetworkBodyBuffer::writeImplementation(const char * data, unsigned int dataSize)
{
	unsigned int bytesAppended = _bodyChunk.append(data, dataSize);
	while (bytesAppended < dataSize) {
		//_response.sendChunk(_bodyChunk, false);
		sendChunk(false);
		_bodyChunk.clear();
		bytesAppended += _bodyChunk.append(data + bytesAppended, dataSize - bytesAppended);
	}
}

void HTTPResponse::NetworkBodyBuffer::clearImplementation()
{
	if (_transferStarted) {
		// TODO
		throw std::logic_error("Can not clear network body buffer after starting data transmission");
	}
	_bodyChunk.clear();
}

/*------------------------------------------------------------------------------
 * HTTPResponse::NullBodyBuffer
------------------------------------------------------------------------------*/

HTTPResponse::NullBodyBuffer::NullBodyBuffer() :
	AbstractBodyBuffer()
{}

//HTTPResponse::NullBodyBuffer::~NullBodyBuffer()
//{}

unsigned int HTTPResponse::NullBodyBuffer::readImplementation(char * buffer, unsigned int bufferSize)
{
	return 0;
}

void HTTPResponse::NullBodyBuffer::writeImplementation(const char * data, unsigned int dataSize)
{}

void HTTPResponse::NullBodyBuffer::clearImplementation()
{}


} // namespace isl

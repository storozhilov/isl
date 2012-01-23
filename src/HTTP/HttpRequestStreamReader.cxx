#include <isl/HttpRequestStreamReader.hxx>
#include <isl/Http.hxx>
#include <isl/HttpError.hxx>
#include <isl/IOError.hxx>
#include <sstream>

namespace isl
{

/*------------------------------------------------------------------------------
 * HttpRequestStreamReader
------------------------------------------------------------------------------*/

HttpRequestStreamReader::HttpRequestStreamReader(AbstractIODevice& device) :
	_device(device),
	_parserState(ParsingRequest),
	_pos(0),
	_line(1),
	_col(1),
	_method(),
	_uri(),
	_version(),
	_headerFieldName(),
	_headerFieldValue(),
	_header(),
	_cookies(),
	_contentLength(0),
	_identityBodyBytesParsed(0),
	_chunkSizeStr(),
	_chunkSize(0),
	_chunkBytesParsed(0),
	_bodyByte(),
	_maxMethodLength(MaxMethodLength),
	_maxUriLength(MaxUriLength),
	_maxVersionLength(MaxVersionLength),
	_maxHeaderFieldNameLength(MaxHeaderFieldNameLength),
	_maxHeaderFieldValueLength(MaxHeaderFieldValueLength)
{}

HttpRequestStreamReader::~HttpRequestStreamReader()
{}

void HttpRequestStreamReader::reset()
{
	_parserState = ParsingRequest;
	_pos = 0;
	_line = 1;
	_col = 1;
	_method.clear();
	_uri.clear();
	_version.clear();
	_headerFieldName.clear();
	_headerFieldValue.clear();
	_header.clear();
	_cookies.clear();
	_contentLength = 0;
	_identityBodyBytesParsed = 0;
	_chunkSizeStr.clear();
	_chunkSize = 0;
	_chunkBytesParsed = 0;
}

unsigned int HttpRequestStreamReader::read(char * buffer, unsigned int bufferSize, const Timeout& timeout)
{
	// Resetting reader/parser if previous request has been completed
	if (requestCompleted()) {
		reset();
	}
	Timeout curTimeout = timeout;
	unsigned int bytesRead = 0;
	bool isFirstReading = true;
	while (bytesRead < (bufferSize - 1) && !requestCompleted()) {
		// Checking parser to be in a valid state
		if (invalidRequest()) {
			throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::InvalidParserState));
		}
		// Fetching next character from the IO-device
		char ch;
		if (!_device.getChar(ch, curTimeout)) {
			if (isFirstReading) {
				// Timeout expired on first reading
				throw Exception(IOError(SOURCE_LOCATION_ARGS, IOError::TimeoutExpired));
			}
			break;
		}
		// Parsing next fetched character
		if (parse(ch)) {
			buffer[bytesRead++] = _bodyByte;
		}
		// Resetting timeout to zero after first reading has been done
		if (isFirstReading) {
			curTimeout = Timeout();
			isFirstReading = false;
		}
		// Updating current position data
		++_pos;
		if (isLineFeed(ch)) {
			++_line;
			_col = 1;
		} else {
			++_col;
		}
	}
	return bytesRead;
}

bool HttpRequestStreamReader::invalidRequest() const
{
	return	_parserState == BadRequest ||
		_parserState == RequestMethodTooLong ||
		_parserState == RequestUriTooLong ||
		_parserState == RequestVersionTooLong ||
		_parserState == RequestHeaderFieldNameTooLong ||
		_parserState == RequestHeaderFieldValueTooLong ||
		_parserState == RequestEntityTooLong ||
		_parserState == InvalidState;
}

bool HttpRequestStreamReader::headerContains(const std::string& fieldName) const
{
	std::pair<Header::const_iterator, Header::const_iterator> range = _header.equal_range(fieldName);
	return range.first != _header.end();
}

bool HttpRequestStreamReader::headerContains(const std::string& fieldName, const std::string& fieldValue) const
{
	std::pair<Header::const_iterator, Header::const_iterator> range = _header.equal_range(fieldName);
	for (Header::const_iterator i = range.first; i != range.second; ++i) {
		if (i->second == fieldValue) {
			return true;
		}
	}
	return false;
}

std::string HttpRequestStreamReader::headerValue(const std::string& fieldName) const
{
	std::pair<Header::const_iterator, Header::const_iterator> range = _header.equal_range(fieldName);
	return range.first == _header.end() ? std::string() : range.first->second;
}

std::list<std::string> HttpRequestStreamReader::headerValues(const std::string& fieldName) const
{
	std::list<std::string> result;
	std::pair<Header::const_iterator, Header::const_iterator> range = _header.equal_range(fieldName);
	for (Header::const_iterator i = range.first; i != range.second; ++i) {
		result.push_back(i->second);
	}
	return result;
}

bool HttpRequestStreamReader::parse(char ch)
{
	bool bodyByteExtracted = false;
	switch (_parserState) {
	case ParsingRequest:
		if (isToken(ch)) {
			_method = ch;
			_parserState = ParsingMethod;
		} else {
			_parserState = BadRequest;
			std::wostringstream msg;
			msg << L"Request method starts with invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
		}
		break;
	case ParsingMethod:
		if (isToken(ch)) {
			if (_method.length() < maxMethodLength()) {
				_method += ch;
			} else {
				_parserState = RequestMethodTooLong;
				Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Request method is too long"));
				throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest));
			}
		} else if (isSpaceOrTab(ch)) {
			_parserState = ParsingMethodUriDelimeter;
		} else {
			_parserState = BadRequest;
			std::wostringstream msg;
			msg << L"Request method contains invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
		}
		break;
	case ParsingMethodUriDelimeter:
		if (isSpaceOrTab(ch)) {
			// Just ignore it
		} else if (isAllowedInUri(ch)) {
			_parserState = ParsingUri;
			_uri = ch;
		} else {
			_parserState = BadRequest;
			std::wostringstream msg;
			msg << L"Request URI starts with invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
		}
		break;
	case ParsingUri:
		if (isAllowedInUri(ch)) {
			if (_uri.length() < maxUriLength()) {
				_uri += ch;
			} else {
				_parserState = RequestUriTooLong;
				Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Request URI is too long"));
				throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::RequestUriTooLong));
			}
		} else if (isSpaceOrTab(ch)) {
			_parserState = ParsingUriVersionDelimeter;
		} else {
			_parserState = BadRequest;
			std::wostringstream msg;
			msg << L"Request URI contains invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
		}
		break;
	case ParsingUriVersionDelimeter:
		if (isSpaceOrTab(ch)) {
			// Just ignore it
		} else if (isAllowedInUri(ch)) {
			_parserState = ParsingVersion;
			_version = ch;
		} else {
			_parserState = BadRequest;
			std::wostringstream msg;
			msg << L"Request HTTP-version starts with invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
		}
		break;
	case ParsingVersion:
		if (isCarriageReturn(ch)) {
			_parserState = ParsingVersionLF;
		} else if (isAllowedInVersion(ch)) {
			if (_version.length() < maxVersionLength()) {
				_version += ch;
			} else {
				_parserState = RequestVersionTooLong;
				Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Request version is too long"));
				throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::RequestVersionTooLong));
			}
		} else {
			_parserState = BadRequest;
			std::wostringstream msg;
			msg << L"Request HTTP-version contains invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
		}
		break;
	case ParsingVersionLF:
		if (isLineFeed(ch)) {
			_parserState = ParsingHeaderField;
		} else {
			_parserState = BadRequest;
			std::wostringstream msg;
			msg << L"Request line's CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
			Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
		}
		break;
	case ParsingHeaderField:
		parseHeaderField(ch, false);
		break;
	case ParsingHeaderFieldName:
		parseHeaderFieldName(ch, false);
		break;
	case ParsingHeaderFieldValue:
		parseHeaderFieldValue(ch, false);
		break;
	case ParsingHeaderFieldValueLF:
		parseHeaderFieldValueLF(ch, false);
		break;
	case ParsingHeaderFieldValueLWS:
		parseHeaderFieldValueLWS(ch, false);
		break;
	case ParsingEndOfHeader:
		if (isLineFeed(ch)) {
			if (headerContains("Transfer-Encoding", "chunked")) {
				_parserState = ParsingChunkSize;
			} else if (headerContains("Content-Length")) {
				// Extracting the content length
				bool contentLengthConversionErrorOccured;
				_contentLength = String::toUnsignedInt(headerValue("Content-Length"), &contentLengthConversionErrorOccured);
				if (contentLengthConversionErrorOccured) {
					_parserState = BadRequest;
					throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, L"Invalid 'Content-Length' header field value"));
				} else if (_contentLength <= 0) {
					_parserState = RequestCompleted;
				} else {
					_parserState = ParsingIdentityBody;
				}
			} else {
				_parserState = RequestCompleted;
			}
		} else {
			_parserState = BadRequest;
			std::wostringstream msg;
			msg << L"Request header's CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
			Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
		}
		break;
	case ParsingIdentityBody:
		_bodyByte = ch;
		++_identityBodyBytesParsed;
		bodyByteExtracted = true;
		if (_identityBodyBytesParsed >= _contentLength) {
			_parserState = RequestCompleted;
		}
		break;
	case ParsingChunkSize:
		if (isHexDigit(ch)) {
			_chunkSizeStr += ch;
		} else {
			if (_chunkSizeStr.empty()) {
				_parserState = BadRequest;
				throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, L"Empty chunk size"));
			}
			bool chunkSizeConversionErrorOccured;
			_chunkSize = String::toUnsignedInt(_chunkSizeStr, &chunkSizeConversionErrorOccured, String::HexBase);
			if (chunkSizeConversionErrorOccured) {
				_parserState = BadRequest;
				throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, L"Invalid chunk size"));
			}
			_chunkBytesParsed = 0;
			_chunkSizeStr.clear();
			if (isCarriageReturn(ch)) {
				_parserState = ParsingChunkSizeLF;
			} else {
				_parserState = ParsingChunkExtension;
			}
		}
		break;
	case ParsingChunkExtension:
		// Just ignore a chunk extension.
		if (isCarriageReturn(ch)) {
			_parserState = ParsingChunkSizeLF;
		}
		break;
	case ParsingChunkSizeLF:
		if (!isLineFeed(ch)) {
			_parserState = BadRequest;
			std::wostringstream msg;
			msg << L"Chunk size's CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
			Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
		}
		_parserState = (_chunkSize > 0) ? ParsingChunk : ParsingTrailerHeaderField;
		break;
	case ParsingChunk:
		_bodyByte = ch;
		++_chunkBytesParsed;
		bodyByteExtracted = true;
		if (_chunkBytesParsed >= _chunkSize) {
			_parserState = ParsingChunkCR;
		}
		break;
	case ParsingChunkCR:
		if (!isCarriageReturn(ch)) {
			_parserState = BadRequest;
			std::wostringstream msg;
			msg << L"Chunk data is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" instead of CR at " << std::dec << _pos << L" position";
			Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
		}
		_parserState = ParsingChunkLF;
		break;
	case ParsingChunkLF:
		if (!isLineFeed(ch)) {
			_parserState = BadRequest;
			std::wostringstream msg;
			msg << L"Chunk data CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
			Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
		}
		_parserState = ParsingChunkSize;
		break;
	case ParsingTrailerHeaderField:
		parseHeaderField(ch, true);
		break;
	case ParsingTrailerHeaderFieldName:
		parseHeaderFieldName(ch, true);
		break;
	case ParsingTrailerHeaderFieldValue:
		parseHeaderFieldValue(ch, true);
		break;
	case ParsingTrailerHeaderFieldValueLF:
		parseHeaderFieldValueLF(ch, true);
		break;
	case ParsingTrailerHeaderFieldValueLWS:
		parseHeaderFieldValueLWS(ch, true);
		break;
	case ParsingFinalLF:
		if (!isLineFeed(ch)) {
			_parserState = BadRequest;
			std::wostringstream msg;
			msg << L"Final CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
			Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
		}
		_parserState = RequestCompleted;
		break;
	default:
		_parserState = InvalidState;
		throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::InvalidParserState));
	}
	return bodyByteExtracted;
}

void HttpRequestStreamReader::appendHeader()
{
	String::trim(_headerFieldName);
	String::trim(_headerFieldValue);
	_header.insert(Header::value_type(_headerFieldName, _headerFieldValue));
	if (_headerFieldName == "Cookie") {
		// Parsing and adding cookies
		int i = 0;
		while (i < _headerFieldValue.length()) {						// I don't like this algorithm, but it works...
			// Passing leading spaces
			while ((i < _headerFieldValue.length()) && String::isSpaceOrTab(_headerFieldValue[i])) {
				++i;
			}
			// Extracting cookie name
			std::string cookieName;
			while ((i < _headerFieldValue.length()) && (_headerFieldValue[i] != '=') && (_headerFieldValue[i] != ';')) {
				cookieName += _headerFieldValue[i];
				++i;
			}
			if (cookieName.empty()) {
				// Finding next cookie;
				while ((i < _headerFieldValue.length()) && (_headerFieldValue[i] != ';')) {
					++i;
				}
				// Passing ';' if found
				if (i < _headerFieldValue.length()) {
					++i;
				}
				continue;
			}
			// Extracting cookie value
			std::string cookieValue;
			if ((i < _headerFieldValue.length()) && (_headerFieldValue[i] == '=')) {
				++i;
				while ((i < _headerFieldValue.length()) && (_headerFieldValue[i] != ';')) {
					cookieValue += _headerFieldValue[i];
					++i;
				}
			}
			// Inserting cookie into HTTP-request
			if (_cookies.find(cookieName) == _cookies.end()) {
				_cookies.insert(Cookies::value_type(cookieName, String::urlDecode(cookieValue)));
			}
			// Passing ';' if found
			if (i < _headerFieldValue.length()) {
				++i;
			}
		}
	}
	_headerFieldName.clear();
	_headerFieldValue.clear();
}

void HttpRequestStreamReader::parseHeaderField(char ch, bool isTrailer)
{
	_headerFieldName.clear();
	_headerFieldValue.clear();
	if (isCarriageReturn(ch)) {
		_parserState = isTrailer ? ParsingFinalLF : ParsingEndOfHeader;
	} else if (ch == ':') {
		// TODO ???????????????????????????????????????????????????????????????????
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValue : ParsingHeaderFieldValue;
	} else if (isAllowedInHeader(ch)) {
		_headerFieldName += ch;
		_parserState = isTrailer ? ParsingTrailerHeaderFieldName : ParsingHeaderFieldName;
	} else {
		_parserState = BadRequest;
		std::wostringstream msg;
		msg << L"Request " << (isTrailer ? "trailer " : "") << "header field starts with invalid character " << std::showbase << std::hex <<
			static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
		Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
	}
}

void HttpRequestStreamReader::parseHeaderFieldName(char ch, bool isTrailer)
{
	if (isCarriageReturn(ch)) {
		Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Request header field is missing ':' separator"));
		throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, L"Request header field is missing ':' separator"));
	} else if (ch == ':') {
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValue : ParsingHeaderFieldValue;
	} else if (isAllowedInHeader(ch)) {
		if (_headerFieldName.length() < maxHeaderFieldNameLength()) {
			_headerFieldName += ch;
		} else {
			_parserState = RequestHeaderFieldNameTooLong;
			Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Request header field name is too long"));
			//throw Exception(HttpError(HttpError::RequestHeaderFieldNameTooLong(), SOURCE_LOCATION_ARGS));
			throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, L"Request header field name is too long"));
		}
	} else {
		_parserState = BadRequest;
		std::wostringstream msg;
		msg << L"Request " << (isTrailer ? "trailer " : "") << "header field name contains invalid character " << std::showbase << std::hex <<
			static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
		Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
	}
}

void HttpRequestStreamReader::parseHeaderFieldValue(char ch, bool isTrailer)
{
	if (isCarriageReturn(ch)) {
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValueLF : ParsingHeaderFieldValueLF;
	} else if (isAllowedInHeader(ch)) {
		if (_headerFieldValue.length() < maxHeaderFieldValueLength()) {
			_headerFieldValue += ch;
		} else {
			_parserState = RequestHeaderFieldValueTooLong;
			Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Request header field value is too long"));
			throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, L"Request header field value is too long"));
		}
	} else {
		_parserState = BadRequest;
		std::wostringstream msg;
		msg << L"Request " << (isTrailer ? "trailer " : "") << "header field value contains invalid character " << std::showbase << std::hex <<
			static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
		Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
	}
}

void HttpRequestStreamReader::parseHeaderFieldValueLF(char ch, bool isTrailer)
{
	if (isLineFeed(ch)) {
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValueLWS : ParsingHeaderFieldValueLWS;
	} else {
		_parserState = BadRequest;
		std::wostringstream msg;
		msg << L"Request " << (isTrailer ? "trailer " : "") << "header field's CR is followed by the invalid character " << std::showbase << std::hex <<
			static_cast<int>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
		Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
	}
}

void HttpRequestStreamReader::parseHeaderFieldValueLWS(char ch, bool isTrailer)
{
	if (isCarriageReturn(ch)) {
		appendHeader();
		_parserState = isTrailer ? ParsingFinalLF : ParsingEndOfHeader;
	} else if (ch == ':') {
		// TODO ?????????????????????????????????????????????????????????????????
		_headerFieldName.clear();
		_headerFieldValue.clear();
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValue : ParsingHeaderFieldValue;
	} else if (isSpaceOrTab(ch)) {
		_headerFieldValue += ' ';
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValue : ParsingHeaderFieldValue;
	} else if (isAllowedInHeader(ch)) {
		appendHeader();
		_headerFieldName += ch;
		_parserState = isTrailer ? ParsingTrailerHeaderFieldName : ParsingHeaderFieldName;
	} else {
		_parserState = BadRequest;
		std::wostringstream msg;
		msg << L"Request " << (isTrailer ? "trailer " : "") << "header field starts with invalid character " << std::showbase << std::hex <<
			static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
		Http::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		throw Exception(HttpError(SOURCE_LOCATION_ARGS, HttpError::BadRequest, msg.str()));
	}
}

} // namespace isl

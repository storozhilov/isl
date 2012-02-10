#include <isl/HttpMessageStreamReader.hxx>
#include <isl/Core.hxx>
#include <isl/Error.hxx>
#include <isl/HttpError.hxx>

namespace isl
{

HttpMessageStreamReader::HttpMessageStreamReader(AbstractIODevice& device) :
	_device(device),
	_parserState(ParsingMessage),
	_isBad(false),
	_parsingError(),
	_pos(0),
	_line(1),
	_col(1),
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
	_maxHeaderFieldNameLength(MaxHeaderFieldNameLength),
	_maxHeaderFieldValueLength(MaxHeaderFieldValueLength)
{}

HttpMessageStreamReader::~HttpMessageStreamReader()
{}

void HttpMessageStreamReader::reset()
{
	_parserState = ParsingMessage;
	_isBad = false;
	_parsingError.clear();
	_pos = 0;
	_line = 1;
	_col = 1;
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

unsigned int HttpMessageStreamReader::read(char * buffer, unsigned int bufferSize, const Timeout& timeout, bool * timeoutExpired)
{
	// Checking parser to be in a valid state
	if (isBad()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, L"Bad HTTP-message detected"));
	}
	// Resetting reader/parser if previous message has been completed
	if (isCompleted()) {
		reset();
	}
	if (timeoutExpired) {
		*timeoutExpired = false;
	}
	Timeout curTimeout = timeout;
	unsigned int bytesRead = 0;
	bool isFirstReading = true;
	while (bytesRead < (bufferSize - 1) && !isCompleted()) {
		// Fetching next character from the IO-device
		char ch;
		if (!_device.getChar(ch, curTimeout)) {
			// Timeout expired
			if (isFirstReading && timeoutExpired) {
				*timeoutExpired = true;
			}
			break;
		}
		// Parsing next fetched character
		if (parse(ch)) {
			buffer[bytesRead++] = _bodyByte;
		}
		if (isBad()) {
			return bytesRead;
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

bool HttpMessageStreamReader::headerContains(const std::string& fieldName) const
{
	std::pair<HttpHeader::const_iterator, HttpHeader::const_iterator> range = _header.equal_range(fieldName);
	return range.first != range.second;
}

bool HttpMessageStreamReader::headerContains(const std::string& fieldName, const std::string& fieldValue) const
{
	std::pair<HttpHeader::const_iterator, HttpHeader::const_iterator> range = _header.equal_range(fieldName);
	for (HttpHeader::const_iterator i = range.first; i != range.second; ++i) {
		if (i->second == fieldValue) {
			return true;
		}
	}
	return false;
}

std::string HttpMessageStreamReader::headerValue(const std::string& fieldName) const
{
	std::pair<HttpHeader::const_iterator, HttpHeader::const_iterator> range = _header.equal_range(fieldName);
	return range.first == range.second ? std::string() : range.first->second;
}

std::list<std::string> HttpMessageStreamReader::headerValues(const std::string& fieldName) const
{
	std::list<std::string> result;
	std::pair<HttpHeader::const_iterator, HttpHeader::const_iterator> range = _header.equal_range(fieldName);
	for (HttpHeader::const_iterator i = range.first; i != range.second; ++i) {
		result.push_back(i->second);
	}
	return result;
}

bool HttpMessageStreamReader::parse(char ch)
{
	bool bodyByteExtracted = false;
	switch (_parserState) {
	case ParsingMessage:
		if (isAllowedInFirstToken(ch)) {
			appendToFirstToken(ch);
			if (!isBad()) {
				_parserState = ParsingFirstToken;
			}
		} else {
			std::wostringstream msg;
			msg << L"HTTP-message starts with invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	case ParsingFirstToken:
		if (isSpaceOrTab(ch)) {
			_parserState = ParsingFirstTokenSP;
		} else if (isAllowedInFirstToken(ch)) {
			appendToFirstToken(ch);
		} else {
			std::wostringstream msg;
			msg << L"First token contains invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	case ParsingFirstTokenSP:
		if (isSpaceOrTab(ch)) {
			// Just ignore it
		} else if (isAllowedInSecondToken(ch)) {
			appendToSecondToken(ch);
			if (!isBad()) {
				_parserState = ParsingSecondToken;
			}
		} else {
			std::wostringstream msg;
			msg << L"Second token starts with invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	case ParsingSecondToken:
		if (isSpaceOrTab(ch)) {
			_parserState = ParsingSecondTokenSP;
		} else if (isAllowedInSecondToken(ch)) {
			appendToSecondToken(ch);
		} else {
			std::wostringstream msg;
			msg << L"Second token contains invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	case ParsingSecondTokenSP:
		if (isSpaceOrTab(ch)) {
			// Just ignore it
		} else if (isAllowedInThirdToken(ch)) {
			appendToThirdToken(ch);
			if (!isBad()) {
				_parserState = ParsingThirdToken;
			}
		} else {
			std::wostringstream msg;
			msg << L"Third token starts with invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	case ParsingThirdToken:
		if (isCarriageReturn(ch)) {
			_parserState = ParsingFirstLineLF;
		} else if (isAllowedInThirdToken(ch)) {
			appendToThirdToken(ch);
		} else {
			std::wostringstream msg;
			msg << L"Third token contains invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	case ParsingFirstLineLF:
		if (isLineFeed(ch)) {
			_parserState = ParsingHeaderField;
		} else {
			std::wostringstream msg;
			msg << L"HTTP-message line's CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
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
					setIsBad(L"Invalid 'Content-Length' header field value");
				} else if (_contentLength <= 0) {
					_parserState = MessageCompleted;
				} else {
					_parserState = ParsingIdentityBody;
				}
			} else {
				_parserState = MessageCompleted;
			}
		} else {
			std::wostringstream msg;
			msg << L"HTTP-message header's CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	case ParsingIdentityBody:
		_bodyByte = ch;
		++_identityBodyBytesParsed;
		bodyByteExtracted = true;
		if (_identityBodyBytesParsed >= _contentLength) {
			_parserState = MessageCompleted;
		}
		break;
	case ParsingChunkSize:
		if (isHexDigit(ch)) {
			_chunkSizeStr += ch;
		} else {
			if (_chunkSizeStr.empty()) {
				setIsBad(L"Empty chunk size");
			} else {
				bool chunkSizeConversionErrorOccured;
				_chunkSize = String::toUnsignedInt(_chunkSizeStr, &chunkSizeConversionErrorOccured, String::HexBase);
				if (chunkSizeConversionErrorOccured) {
					setIsBad(L"Invalid chunk size");
				} else {
					_chunkBytesParsed = 0;
					_chunkSizeStr.clear();
					if (isCarriageReturn(ch)) {
						_parserState = ParsingChunkSizeLF;
					} else {
						_parserState = ParsingChunkExtension;
					}
				}
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
		if (isLineFeed(ch)) {
			_parserState = (_chunkSize > 0) ? ParsingChunk : ParsingTrailerHeaderField;
		} else {
			std::wostringstream msg;
			msg << L"Chunk size's CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
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
		if (isCarriageReturn(ch)) {
			_parserState = ParsingChunkLF;
		} else {
			std::wostringstream msg;
			msg << L"Chunk data is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" instead of CR at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	case ParsingChunkLF:
		if (isLineFeed(ch)) {
			_parserState = ParsingChunkSize;
		} else {
			std::wostringstream msg;
			msg << L"Chunk data CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
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
		if (isLineFeed(ch)) {
			_parserState = MessageCompleted;
		} else {
			std::wostringstream msg;
			msg << L"Final CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	default:
		throw Exception(Error(SOURCE_LOCATION_ARGS, L"Invalid parser state"));
	}
	return bodyByteExtracted;
}

void HttpMessageStreamReader::appendHeader()
{
	String::trim(_headerFieldName);
	String::trim(_headerFieldValue);
	_header.insert(HttpHeader::value_type(_headerFieldName, _headerFieldValue));
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
			// Inserting cookie into HTTP-message
			if (_cookies.find(cookieName) == _cookies.end()) {
				_cookies.insert(HttpCookies::value_type(cookieName, String::urlDecode(cookieValue)));
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

void HttpMessageStreamReader::parseHeaderField(char ch, bool isTrailer)
{
	_headerFieldName.clear();
	_headerFieldValue.clear();
	if (isCarriageReturn(ch)) {
		_parserState = isTrailer ? ParsingFinalLF : ParsingEndOfHeader;
	} else if (ch == ':') {
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Empty HTTP-message header field name"));
		setIsBad(L"Empty HTTP-message header field name");
	} else if (isAllowedInHeader(ch)) {
		_headerFieldName += ch;
		_parserState = isTrailer ? ParsingTrailerHeaderFieldName : ParsingHeaderFieldName;
	} else {
		std::wostringstream msg;
		msg << L"HTTP-message " << (isTrailer ? "trailer " : "") << "header field starts with invalid character " << std::showbase << std::hex <<
			static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		setIsBad(msg.str());
	}
}

void HttpMessageStreamReader::parseHeaderFieldName(char ch, bool isTrailer)
{
	if (isCarriageReturn(ch)) {
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"HTTP-message header field is missing ':' separator"));
		setIsBad(L"HTTP-message header field is missing ':' separator");
	} else if (ch == ':') {
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValue : ParsingHeaderFieldValue;
	} else if (isAllowedInHeader(ch)) {
		if (_headerFieldName.length() < maxHeaderFieldNameLength()) {
			_headerFieldName += ch;
		} else {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"HTTP-message header field name is too long"));
			setIsBad(L"HTTP-message header field name is too long");
		}
	} else {
		std::wostringstream msg;
		msg << L"HTTP-message " << (isTrailer ? "trailer " : "") << "header field name contains invalid character " << std::showbase << std::hex <<
			static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		setIsBad(msg.str());
	}
}

void HttpMessageStreamReader::parseHeaderFieldValue(char ch, bool isTrailer)
{
	if (isCarriageReturn(ch)) {
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValueLF : ParsingHeaderFieldValueLF;
	} else if (isAllowedInHeader(ch)) {
		if (_headerFieldValue.length() < maxHeaderFieldValueLength()) {
			_headerFieldValue += ch;
		} else {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"HTTP-message header field value is too long"));
			setIsBad(L"HTTP-message header field value is too long");
		}
	} else {
		std::wostringstream msg;
		msg << L"HTTP-message " << (isTrailer ? "trailer " : "") << "header field value contains invalid character " << std::showbase << std::hex <<
			static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		setIsBad(msg.str());
	}
}

void HttpMessageStreamReader::parseHeaderFieldValueLF(char ch, bool isTrailer)
{
	if (isLineFeed(ch)) {
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValueLWS : ParsingHeaderFieldValueLWS;
	} else {
		std::wostringstream msg;
		msg << L"HTTP-message " << (isTrailer ? "trailer " : "") << "header field's CR is followed by the invalid character " << std::showbase << std::hex <<
			static_cast<int>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		setIsBad(msg.str());
	}
}

void HttpMessageStreamReader::parseHeaderFieldValueLWS(char ch, bool isTrailer)
{
	if (isCarriageReturn(ch)) {
		appendHeader();
		_parserState = isTrailer ? ParsingFinalLF : ParsingEndOfHeader;
	} else if (ch == ':') {
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Empty HTTP-message header field name"));
		setIsBad(L"Empty HTTP-message header field name");
	} else if (isSpaceOrTab(ch)) {
		_headerFieldValue += ' ';
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValue : ParsingHeaderFieldValue;
	} else if (isAllowedInHeader(ch)) {
		appendHeader();
		_headerFieldName += ch;
		_parserState = isTrailer ? ParsingTrailerHeaderFieldName : ParsingHeaderFieldName;
	} else {
		std::wostringstream msg;
		msg << L"HTTP-message " << (isTrailer ? "trailer " : "") << "header field starts with invalid character " << std::showbase << std::hex <<
			static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		setIsBad(msg.str());
	}
}

} // namespace isl

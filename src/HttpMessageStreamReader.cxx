#include <isl/HttpMessageStreamReader.hxx>
#include <isl/Char.hxx>
#include <isl/Core.hxx>
#include <isl/Error.hxx>

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
	_contentLength = 0;
	_identityBodyBytesParsed = 0;
	_chunkSizeStr.clear();
	_chunkSize = 0;
	_chunkBytesParsed = 0;
}

size_t HttpMessageStreamReader::read(char * buffer, size_t bufferSize, const Timeout& timeout, bool * timeoutExpired)
{
	// Checking parser to be in a valid state
	if (isBad()) {
		//throw Exception(Error(SOURCE_LOCATION_ARGS, L"Bad HTTP-message detected"));
		throw Exception(Error(SOURCE_LOCATION_ARGS, _parsingError));
	}
	// Resetting reader/parser if previous message has been completed
	if (isCompleted()) {
		reset();
	}
	if (timeoutExpired) {
		*timeoutExpired = false;
	}
	size_t bytesRead = 0;
	while (bytesRead < (bufferSize - 1) && !isCompleted()) {
		// Fetching next character from the IO-device
		char ch;
		if (!_device.getChar(ch, timeout)) {
			// Timeout expired
			if (timeoutExpired) {
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
		// Updating current position data
		++_pos;
		if (Char::isLineFeed(ch)) {
			++_line;
			_col = 1;
		} else {
			++_col;
		}
	}
	return bytesRead;
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
				static_cast<unsigned char>(ch) << L" at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	case ParsingFirstToken:
		if (Char::isSpaceOrTab(ch)) {
			_parserState = ParsingFirstTokenSP;
		} else if (isAllowedInFirstToken(ch)) {
			appendToFirstToken(ch);
		} else {
			std::wostringstream msg;
			msg << L"First token contains invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << L" at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	case ParsingFirstTokenSP:
		if (Char::isSpaceOrTab(ch)) {
			// Just ignore it
		} else if (isAllowedInSecondToken(ch)) {
			appendToSecondToken(ch);
			if (!isBad()) {
				_parserState = ParsingSecondToken;
			}
		} else {
			std::wostringstream msg;
			msg << L"Second token starts with invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << L" at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	case ParsingSecondToken:
		if (Char::isSpaceOrTab(ch)) {
			_parserState = ParsingSecondTokenSP;
		} else if (isAllowedInSecondToken(ch)) {
			appendToSecondToken(ch);
		} else {
			std::wostringstream msg;
			msg << L"Second token contains invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << L" at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	case ParsingSecondTokenSP:
		if (Char::isSpaceOrTab(ch)) {
			// Just ignore it
		} else if (isAllowedInThirdToken(ch)) {
			appendToThirdToken(ch);
			if (!isBad()) {
				_parserState = ParsingThirdToken;
			}
		} else {
			std::wostringstream msg;
			msg << L"Third token starts with invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << L" at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	case ParsingThirdToken:
		if (Char::isCarriageReturn(ch)) {
			_parserState = ParsingFirstLineLF;
		} else if (isAllowedInThirdToken(ch)) {
			appendToThirdToken(ch);
		} else {
			std::wostringstream msg;
			msg << L"Third token contains invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << L" at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	case ParsingFirstLineLF:
		if (Char::isLineFeed(ch)) {
			_parserState = ParsingHeaderField;
		} else {
			std::wostringstream msg;
			msg << L"HTTP-message line's CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
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
		if (Char::isLineFeed(ch)) {
			if (Http::hasParam(_header, "Transfer-Encoding", "chunked")) {
				_parserState = ParsingChunkSize;
			} else if (Http::hasParam(_header, "Content-Length")) {
				// Extracting the content length
				bool contentLengthConversionErrorOccured;
				_contentLength = String::toUnsignedInt(Http::paramValue(_header, "Content-Length"), &contentLengthConversionErrorOccured);
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
				static_cast<unsigned char>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
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
		if (Char::isHexDigit(ch)) {
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
					if (Char::isCarriageReturn(ch)) {
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
		if (Char::isCarriageReturn(ch)) {
			_parserState = ParsingChunkSizeLF;
		}
		break;
	case ParsingChunkSizeLF:
		if (Char::isLineFeed(ch)) {
			_parserState = (_chunkSize > 0) ? ParsingChunk : ParsingTrailerHeaderField;
		} else {
			std::wostringstream msg;
			msg << L"Chunk size's CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
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
		if (Char::isCarriageReturn(ch)) {
			_parserState = ParsingChunkLF;
		} else {
			std::wostringstream msg;
			msg << L"Chunk data is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << L" instead of CR at " << std::dec << _pos << L" position";
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(msg.str());
		}
		break;
	case ParsingChunkLF:
		if (Char::isLineFeed(ch)) {
			_parserState = ParsingChunkSize;
		} else {
			std::wostringstream msg;
			msg << L"Chunk data CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
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
		if (Char::isLineFeed(ch)) {
			_parserState = MessageCompleted;
		} else {
			std::wostringstream msg;
			msg << L"Final CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
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
	_header.insert(Http::Params::value_type(_headerFieldName, _headerFieldValue));
	_headerFieldName.clear();
	_headerFieldValue.clear();
}

void HttpMessageStreamReader::parseHeaderField(char ch, bool isTrailer)
{
	_headerFieldName.clear();
	_headerFieldValue.clear();
	if (Char::isCarriageReturn(ch)) {
		_parserState = isTrailer ? ParsingFinalLF : ParsingEndOfHeader;
	} else if (ch == ':') {
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Empty HTTP-message header field name"));
		setIsBad(L"Empty HTTP-message header field name");
	} else if (Http::isToken(ch)) {
		_headerFieldName += ch;
		_parserState = isTrailer ? ParsingTrailerHeaderFieldName : ParsingHeaderFieldName;
	} else {
		std::wostringstream msg;
		msg << L"HTTP-message " << (isTrailer ? "trailer " : "") << "header field starts with invalid character " << std::showbase << std::hex <<
			static_cast<unsigned char>(ch) << L" at " << std::dec << _pos << L" position";
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		setIsBad(msg.str());
	}
}

void HttpMessageStreamReader::parseHeaderFieldName(char ch, bool isTrailer)
{
	if (Char::isCarriageReturn(ch)) {
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"HTTP-message header field is missing ':' separator"));
		setIsBad(L"HTTP-message header field is missing ':' separator");
	} else if (ch == ':') {
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValue : ParsingHeaderFieldValue;
	} else if (Http::isToken(ch)) {
		if (_headerFieldName.length() < maxHeaderFieldNameLength()) {
			_headerFieldName += ch;
		} else {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"HTTP-message header field name is too long"));
			setIsBad(L"HTTP-message header field name is too long");
		}
	} else {
		std::wostringstream msg;
		msg << L"HTTP-message " << (isTrailer ? "trailer " : "") << "header field name contains invalid character " << std::showbase << std::hex <<
			static_cast<unsigned char>(ch) << L" at " << std::dec << _pos << L" position";
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		setIsBad(msg.str());
	}
}

void HttpMessageStreamReader::parseHeaderFieldValue(char ch, bool isTrailer)
{
	if (Char::isCarriageReturn(ch)) {
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValueLF : ParsingHeaderFieldValueLF;
	} else if (!Http::isControl(ch)) {
		if (_headerFieldValue.length() < maxHeaderFieldValueLength()) {
			_headerFieldValue += ch;
		} else {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"HTTP-message header field value is too long"));
			setIsBad(L"HTTP-message header field value is too long");
		}
	} else {
		std::wostringstream msg;
		msg << L"HTTP-message " << (isTrailer ? "trailer " : "") << "header field value contains invalid character " << std::showbase << std::hex <<
			static_cast<unsigned char>(ch) << L" at " << std::dec << _pos << L" position";
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		setIsBad(msg.str());
	}
}

void HttpMessageStreamReader::parseHeaderFieldValueLF(char ch, bool isTrailer)
{
	if (Char::isLineFeed(ch)) {
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValueLWS : ParsingHeaderFieldValueLWS;
	} else {
		std::wostringstream msg;
		msg << L"HTTP-message " << (isTrailer ? "trailer " : "") << "header field's CR is followed by the invalid character " << std::showbase << std::hex <<
			static_cast<unsigned char>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		setIsBad(msg.str());
	}
}

void HttpMessageStreamReader::parseHeaderFieldValueLWS(char ch, bool isTrailer)
{
	if (Char::isCarriageReturn(ch)) {
		appendHeader();
		_parserState = isTrailer ? ParsingFinalLF : ParsingEndOfHeader;
	} else if (ch == ':') {
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Empty HTTP-message header field name"));
		setIsBad(L"Empty HTTP-message header field name");
	} else if (Char::isSpaceOrTab(ch)) {
		_headerFieldValue += ' ';
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValue : ParsingHeaderFieldValue;
	} else if (Http::isToken(ch)) {
		appendHeader();
		_headerFieldName += ch;
		_parserState = isTrailer ? ParsingTrailerHeaderFieldName : ParsingHeaderFieldName;
	} else {
		std::wostringstream msg;
		msg << L"HTTP-message " << (isTrailer ? "trailer " : "") << "header field starts with invalid character " << std::showbase << std::hex <<
			static_cast<unsigned char>(ch) << L" at " << std::dec << _pos << L" position";
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		setIsBad(msg.str());
	}
}

} // namespace isl

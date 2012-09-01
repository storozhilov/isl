#include <isl/common.hxx>
#include <isl/HttpMessageStreamReader.hxx>
#include <isl/LogMessage.hxx>
#include <isl/Char.hxx>
#include <isl/Error.hxx>

namespace isl
{

HttpMessageStreamReader::HttpMessageStreamReader(AbstractIODevice& device, size_t bufferSize) :
	_device(device),
	_buffer(bufferSize),
	_bufferSize(0),
	_bufferPosition(0),
	_parserState(ParsingMessage),
	_errorAutoPtr(),
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
	_errorAutoPtr.reset();
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

size_t HttpMessageStreamReader::read(char * bodyBuffer, size_t bodyBufferSize, const Timeout& timeout, size_t * bytesReadFromDevice)
{
	// Checking parser to be in a valid state
	if (isBad()) {
		throw Exception(*_errorAutoPtr.get());
	}
	// Resetting reader/parser if previous message has been completed
	if (isCompleted()) {
		reset();
	}
	if (bytesReadFromDevice) {
		*bytesReadFromDevice = 0;
	}
	if (bodyBufferSize <= 0) {
		return 0;
	}
	size_t bodyBytesRead = 0;
	// Parsing the rest of the data in the buffer
	while (_bufferPosition < _bufferSize) {
		char ch = _buffer[_bufferPosition++];
		if (parse(ch)) {
			bodyBuffer[bodyBytesRead++] = _bodyByte;
		}
		if (isCompleted() || isBad() || bodyBytesRead >= bodyBufferSize) {
			return bodyBytesRead;
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
	// Reading available data from the device
	bool isFirstReading = true;
	while (true) {
		_bufferPosition = 0;
		_bufferSize = _device.read(&_buffer[0], _buffer.size(), isFirstReading ? timeout : Timeout());
		if (_bufferSize <= 0) {
			return bodyBytesRead;
		}
		isFirstReading = false;
		if (bytesReadFromDevice) {
			*bytesReadFromDevice += _bufferSize;
		}
		// Parsing data fetched from the device
		while (_bufferPosition < _bufferSize) {
			char ch = _buffer[_bufferPosition++];
			if (parse(ch)) {
				bodyBuffer[bodyBytesRead++] = _bodyByte;
			}
			if (isCompleted() || isBad() || bodyBytesRead >= bodyBufferSize) {
				return bodyBytesRead;
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
	}
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
			std::ostringstream msg;
			msg << "HTTP-message starts with invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << " at " << std::dec << _pos << " position";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
		}
		break;
	case ParsingFirstToken:
		if (Char::isSpaceOrTab(ch)) {
			_parserState = ParsingFirstTokenSP;
		} else if (isAllowedInFirstToken(ch)) {
			appendToFirstToken(ch);
		} else {
			std::ostringstream msg;
			msg << "First token contains invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << " at " << std::dec << _pos << " position";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
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
			std::ostringstream msg;
			msg << "Second token starts with invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << " at " << std::dec << _pos << " position";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
		}
		break;
	case ParsingSecondToken:
		if (Char::isSpaceOrTab(ch)) {
			_parserState = ParsingSecondTokenSP;
		} else if (isAllowedInSecondToken(ch)) {
			appendToSecondToken(ch);
		} else {
			std::ostringstream msg;
			msg << "Second token contains invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << " at " << std::dec << _pos << " position";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
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
			std::ostringstream msg;
			msg << "Third token starts with invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << " at " << std::dec << _pos << " position";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
		}
		break;
	case ParsingThirdToken:
		if (Char::isCarriageReturn(ch)) {
			_parserState = ParsingFirstLineLF;
		} else if (isAllowedInThirdToken(ch)) {
			appendToThirdToken(ch);
		} else {
			std::ostringstream msg;
			msg << "Third token contains invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << " at " << std::dec << _pos << " position";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
		}
		break;
	case ParsingFirstLineLF:
		if (Char::isLineFeed(ch)) {
			_parserState = ParsingHeaderField;
		} else {
			std::ostringstream msg;
			msg << "HTTP-message line's CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << " instead of LF at " << std::dec << _pos << " position";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
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
					setIsBad(Error(SOURCE_LOCATION_ARGS, "Invalid 'Content-Length' header field value"));
				} else if (_contentLength <= 0) {
					_parserState = MessageCompleted;
				} else {
					_parserState = ParsingIdentityBody;
				}
			} else {
				_parserState = MessageCompleted;
			}
		} else {
			std::ostringstream msg;
			msg << "HTTP-message header's CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << " instead of LF at " << std::dec << _pos << " position";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
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
				setIsBad(Error(SOURCE_LOCATION_ARGS, "Empty chunk size"));
			} else {
				bool chunkSizeConversionErrorOccured;
				_chunkSize = String::toUnsignedInt(_chunkSizeStr, &chunkSizeConversionErrorOccured, String::HexBase);
				if (chunkSizeConversionErrorOccured) {
					setIsBad(Error(SOURCE_LOCATION_ARGS, "Invalid chunk size"));
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
			std::ostringstream msg;
			msg << "Chunk size's CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << " instead of LF at " << std::dec << _pos << " position";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
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
			std::ostringstream msg;
			msg << "Chunk data is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << " instead of CR at " << std::dec << _pos << " position";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
		}
		break;
	case ParsingChunkLF:
		if (Char::isLineFeed(ch)) {
			_parserState = ParsingChunkSize;
		} else {
			std::ostringstream msg;
			msg << "Chunk data CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << " instead of LF at " << std::dec << _pos << " position";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
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
			std::ostringstream msg;
			msg << "Final CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<unsigned char>(ch) << " instead of LF at " << std::dec << _pos << " position";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
		}
		break;
	default:
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Invalid parser state"));
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
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Empty HTTP-message header field name"));
		setIsBad(Error(SOURCE_LOCATION_ARGS, "Empty HTTP-message header field name"));
	} else if (Http::isToken(ch)) {
		_headerFieldName += ch;
		_parserState = isTrailer ? ParsingTrailerHeaderFieldName : ParsingHeaderFieldName;
	} else {
		std::ostringstream msg;
		msg << "HTTP-message " << (isTrailer ? "trailer " : "") << "header field starts with invalid character " << std::showbase << std::hex <<
			static_cast<unsigned char>(ch) << " at " << std::dec << _pos << " position";
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
	}
}

void HttpMessageStreamReader::parseHeaderFieldName(char ch, bool isTrailer)
{
	if (Char::isCarriageReturn(ch)) {
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "HTTP-message header field is missing ':' separator"));
		setIsBad(Error(SOURCE_LOCATION_ARGS, "HTTP-message header field is missing ':' separator"));
	} else if (ch == ':') {
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValue : ParsingHeaderFieldValue;
	} else if (Http::isToken(ch)) {
		if (_headerFieldName.length() < maxHeaderFieldNameLength()) {
			_headerFieldName += ch;
		} else {
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "HTTP-message header field name is too long"));
			setIsBad(Error(SOURCE_LOCATION_ARGS, "HTTP-message header field name is too long"));
		}
	} else {
		std::ostringstream msg;
		msg << "HTTP-message " << (isTrailer ? "trailer " : "") << "header field name contains invalid character " << std::showbase << std::hex <<
			static_cast<unsigned char>(ch) << " at " << std::dec << _pos << " position";
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
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
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "HTTP-message header field value is too long"));
			setIsBad(Error(SOURCE_LOCATION_ARGS, "HTTP-message header field value is too long"));
		}
	} else {
		std::ostringstream msg;
		msg << "HTTP-message " << (isTrailer ? "trailer " : "") << "header field value contains invalid character " << std::showbase << std::hex <<
			static_cast<unsigned char>(ch) << " at " << std::dec << _pos << " position";
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
	}
}

void HttpMessageStreamReader::parseHeaderFieldValueLF(char ch, bool isTrailer)
{
	if (Char::isLineFeed(ch)) {
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValueLWS : ParsingHeaderFieldValueLWS;
	} else {
		std::ostringstream msg;
		msg << "HTTP-message " << (isTrailer ? "trailer " : "") << "header field's CR is followed by the invalid character " << std::showbase << std::hex <<
			static_cast<unsigned char>(ch) << " instead of LF at " << std::dec << _pos << " position";
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
	}
}

void HttpMessageStreamReader::parseHeaderFieldValueLWS(char ch, bool isTrailer)
{
	if (Char::isCarriageReturn(ch)) {
		appendHeader();
		_parserState = isTrailer ? ParsingFinalLF : ParsingEndOfHeader;
	} else if (ch == ':') {
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Empty HTTP-message header field name"));
		setIsBad(Error(SOURCE_LOCATION_ARGS, "Empty HTTP-message header field name"));
	} else if (Char::isSpaceOrTab(ch)) {
		_headerFieldValue += ' ';
		_parserState = isTrailer ? ParsingTrailerHeaderFieldValue : ParsingHeaderFieldValue;
	} else if (Http::isToken(ch)) {
		appendHeader();
		_headerFieldName += ch;
		_parserState = isTrailer ? ParsingTrailerHeaderFieldName : ParsingHeaderFieldName;
	} else {
		std::ostringstream msg;
		msg << "HTTP-message " << (isTrailer ? "trailer " : "") << "header field starts with invalid character " << std::showbase << std::hex <<
			static_cast<unsigned char>(ch) << " at " << std::dec << _pos << " position";
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		setIsBad(Error(SOURCE_LOCATION_ARGS, msg.str()));
	}
}

} // namespace isl

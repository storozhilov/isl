#include <isl/HttpMessageParser.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <isl/Char.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>

namespace isl
{

//------------------------------------------------------------------------------
// HttpMessageParser
//------------------------------------------------------------------------------

HttpMessageParser::HttpMessageParser(size_t maxFirstTokenLength, size_t maxSecondTokenLength, size_t maxThirdTokenLength,
		size_t maxHeaderNameLength, size_t maxHeaderValueLength, size_t maxHeadersAmount) :
	_state(ParsingMessage),
	_errorAutoPtr(),
	_pos(0),
	_line(1),
	_col(1),
	_firstToken(),
	_secondToken(),
	_thirdToken(),
	_headerFieldName(),
	_headerFieldValue(),
	_headers(),
	_contentLength(0),
	_identityBodyBytesParsed(0),
	_chunkSizeStr(),
	_chunkSize(0),
	_chunkBytesParsed(0),
	_maxFirstTokenLength(maxFirstTokenLength),
	_maxSecondTokenLength(maxSecondTokenLength),
	_maxThirdTokenLength(maxThirdTokenLength),
	_maxHeaderNameLength(maxHeaderNameLength),
	_maxHeaderValueLength(maxHeaderValueLength),
	_maxHeadersAmount(maxHeadersAmount)
{}

HttpMessageParser::~HttpMessageParser()
{}

bool HttpMessageParser::parse(char ch)
{
	if (isBad()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, ch, _pos, _line, _col, "HTTP-message parser is in error state - reset needed"));
	}
	// Resetting the parser if the previous message has been completely parsed.
	if (_state == MessageCompleted) {
		reset();
	}
	// Parsing the character
	bool bodyByteExtracted = false;
	switch (_state) {
	case ParsingMessage:
		if (Char::isSpaceOrTab(ch)) {
			// Just ignore it
		} else if (Http::isChar(ch) && !Http::isControl(ch)) {
			// First token is empty -> no length check
			_firstToken += ch;
			_state = ParsingFirstToken;
		} else {
			setIsBad(ch, "HTTP-message starts with the invalid character");
		}
		break;
	case ParsingFirstToken:
		if (Char::isSpaceOrTab(ch)) {
			onFirstTokenParsed(_firstToken);
			if (!isBad()) {
				_state = ParsingFirstTokenSP;
			}
		} else if (Http::isChar(ch) && !Http::isControl(ch)) {
			if (_firstToken.length() >= _maxFirstTokenLength) {
				setIsBad(ch, "First token is too long");
			} else {
				_firstToken += ch;
			}
		} else {
			setIsBad(ch, "Invalid character in first token");
		}
		break;
	case ParsingFirstTokenSP:
		if (Char::isSpaceOrTab(ch)) {
			// Just ignore it
		} else if (Http::isChar(ch) && !Http::isControl(ch)) {
			// Second token is empty -> no length check
			_secondToken += ch;
			_state = ParsingSecondToken;
		} else {
			setIsBad(ch, "Second token starts with the invalid character");
		}
		break;
	case ParsingSecondToken:
		if (Char::isSpaceOrTab(ch)) {
			onSecondTokenParsed(_secondToken);
			if (!isBad()) {
				_state = ParsingSecondTokenSP;
			}
		} else if (Http::isChar(ch) && !Http::isControl(ch)) {
			if (_secondToken.length() >= _maxSecondTokenLength) {
				setIsBad(ch, "Second token is too long");
			} else {
				_secondToken += ch;
			}
		} else {
			setIsBad(ch, "Invalid character in second token");
		}
		break;
	case ParsingSecondTokenSP:
		if (Char::isSpaceOrTab(ch)) {
			// Just ignore it
		} else if (Http::isChar(ch) && !Http::isControl(ch)) {
			// Third token is empty -> no length check
			_thirdToken += ch;
			_state = ParsingThirdToken;
		} else {
			setIsBad(ch, "Third token starts with invalid character");
		}
		break;
	case ParsingThirdToken:
		if (Char::isCarriageReturn(ch)) {
			onThirdTokenParsed(_thirdToken);
			if (!isBad()) {
				_state = ParsingFirstLineLF;
			}
		} else if (Http::isChar(ch) && !Http::isControl(ch)) {
			if (_thirdToken.length() >= _maxThirdTokenLength) {
				setIsBad(ch, "Third token is too long");
			} else {
				_thirdToken += ch;
			} 
		} else {
			setIsBad(ch, "Invalid character in third token");
		}
		break;
	case ParsingFirstLineLF:
		if (Char::isLineFeed(ch)) {
			_state = ParsingHeader;
		} else {
			setIsBad(ch, "First line's CR is followed by the invalid character");
		}
		break;
	case ParsingHeader:
		parseHeader(ch, false);
		break;
	case ParsingHeaderName:
		parseHeaderName(ch, false);
		break;
	case ParsingHeaderValue:
		parseHeaderValue(ch, false);
		break;
	case ParsingHeaderValueLF:
		parseHeaderValueLF(ch, false);
		break;
	case ParsingHeaderValueLWS:
		parseHeaderValueLWS(ch, false);
		break;
	case ParsingEndOfHeader:
		if (Char::isLineFeed(ch)) {
			if (Http::hasHeader(_headers, "Transfer-Encoding", "chunked")) {
				_state = ParsingChunkSize;
			} else if (Http::hasHeader(_headers, "Content-Length")) {
				// Extracting the content length
				bool contentLengthConversionErrorOccured;
				_contentLength = String::toUnsignedInt(Http::headerValue(_headers, "Content-Length"), &contentLengthConversionErrorOccured);
				if (contentLengthConversionErrorOccured) {
					std::ostringstream msg;
					msg << "Invalid 'Content-Length' header field unsigned integer value: " << Http::headerValue(_headers, "Content-Length");
					setIsBad(ch, msg.str());
				} else if (_contentLength <= 0) {
					_state = MessageCompleted;
				} else {
					_state = ParsingIdentityBody;
				}
			} else {
				_state = MessageCompleted;
			}
		} else {
			setIsBad(ch, "Header's CR is followed by the invalid character");
		}
		break;
	case ParsingIdentityBody:
		++_identityBodyBytesParsed;
		bodyByteExtracted = true;
		if (_identityBodyBytesParsed >= _contentLength) {
			_state = MessageCompleted;
		}
		break;
	case ParsingChunkSize:
		if (Char::isHexDigit(ch)) {
			_chunkSizeStr += ch;
		} else {
			if (_chunkSizeStr.empty()) {
				setIsBad(ch, "Empty chunk size");
				setIsBad(ch, "");
			} else {
				bool chunkSizeConversionErrorOccured;
				_chunkSize = String::toUnsignedInt(_chunkSizeStr, &chunkSizeConversionErrorOccured, String::HexBase);
				if (chunkSizeConversionErrorOccured) {
					std::ostringstream msg;
					msg << "Invalid chunk size unsigned integer value: " << _chunkSizeStr;
					setIsBad(ch, msg.str());
				} else {
					_chunkBytesParsed = 0;
					_chunkSizeStr.clear();
					if (Char::isCarriageReturn(ch)) {
						_state = ParsingChunkSizeLF;
					} else {
						_state = ParsingChunkExtension;
					}
				}
			}
		}
		break;
	case ParsingChunkExtension:
		// Just ignore a chunk extension.
		if (Char::isCarriageReturn(ch)) {
			_state = ParsingChunkSizeLF;
		}
		break;
	case ParsingChunkSizeLF:
		if (Char::isLineFeed(ch)) {
			_state = (_chunkSize > 0) ? ParsingChunk : ParsingTrailerHeader;
		} else {
			setIsBad(ch, "Chunk size's CR is followed by the invalid character");
		}
		break;
	case ParsingChunk:
		++_chunkBytesParsed;
		bodyByteExtracted = true;
		if (_chunkBytesParsed >= _chunkSize) {
			_state = ParsingChunkCR;
		}
		break;
	case ParsingChunkCR:
		if (Char::isCarriageReturn(ch)) {
			_state = ParsingChunkLF;
		} else {
			setIsBad(ch, "Chunk data is followed by the invalid character");
		}
		break;
	case ParsingChunkLF:
		if (Char::isLineFeed(ch)) {
			_state = ParsingChunkSize;
		} else {
			setIsBad(ch, "Chunk data CR is followed by the invalid character");
		}
		break;
	case ParsingTrailerHeader:
		parseHeader(ch, true);
		break;
	case ParsingTrailerHeaderName:
		parseHeaderName(ch, true);
		break;
	case ParsingTrailerHeaderValue:
		parseHeaderValue(ch, true);
		break;
	case ParsingTrailerHeaderValueLF:
		parseHeaderValueLF(ch, true);
		break;
	case ParsingTrailerHeaderValueLWS:
		parseHeaderValueLWS(ch, true);
		break;
	case ParsingFinalLF:
		if (Char::isLineFeed(ch)) {
			_state = MessageCompleted;
		} else {
			setIsBad(ch, "Final CR is followed by the invalid character");
		}
		break;
	default:
		throw Exception(Error(SOURCE_LOCATION_ARGS, ch, _pos, _line, _col, "Invalid parser state"));
	}
	// Updating current position data
	++_pos;
	if (Char::isLineFeed(ch)) {
		++_line;
		_col = 1;
	} else {
		++_col;
	}
	return bodyByteExtracted;
}

bool HttpMessageParser::bodyExpected() const
{
	return _state == ParsingIdentityBody || _state == ParsingChunk;
}

std::pair<size_t, size_t> HttpMessageParser::parse(const char * parseBuffer, size_t parseBufferSize, char * bodyBuffer, size_t bodyBufferSize)
{
	size_t bytesParsed = 0;
	size_t bodyBytes = 0;
	while (bytesParsed < parseBufferSize) {
		if (bodyExpected() && (bodyBytes >= bodyBufferSize)) {
			// Body buffer has no space for body byte
			break;
		}
		char ch = *(parseBuffer + bytesParsed++);
		if (parse(ch)) {
			*(bodyBuffer + bodyBytes++) = ch;
		}
		if (isCompleted() || isBad()) {
			break;
		}
	}
	return std::pair<size_t, size_t>(bytesParsed, bodyBytes);
}

size_t HttpMessageParser::parse(const char * parseBuffer, size_t parseBufferSize, std::ostream& os)
{
	size_t bytesParsed = 0;
	while (bytesParsed < parseBufferSize) {
		char ch = *(parseBuffer + bytesParsed++);
		if (parse(ch)) {
			os.put(ch);
		}
		if (isCompleted() || isBad()) {
			break;
		}
	}
	return bytesParsed;
}

void HttpMessageParser::reset()
{
	_state = ParsingMessage;
	_errorAutoPtr.reset();
	_pos = 0;
	_line = 1;
	_col = 1;
	_firstToken.clear(),
	_secondToken.clear(),
	_thirdToken.clear(),
	_headerFieldName.clear();
	_headerFieldValue.clear();
	_headers.clear();
	_contentLength = 0;
	_identityBodyBytesParsed = 0;
	_chunkSizeStr.clear();
	_chunkSize = 0;
	_chunkBytesParsed = 0;
}

void HttpMessageParser::setIsBad(char ch, const std::string& errMsg)
{
	_errorAutoPtr.reset(new Error(SOURCE_LOCATION_ARGS, ch, _pos, _line, _col, errMsg));
}

void HttpMessageParser::appendHeader(char ch)
{
	if (_headers.size() >= _maxHeadersAmount) {
		setIsBad(ch, "Too many headers");
		return;
	}
	String::trim(_headerFieldName);
	String::trim(_headerFieldValue);
	_headers.insert(Http::Headers::value_type(_headerFieldName, _headerFieldValue));
	_headerFieldName.clear();
	_headerFieldValue.clear();
}

void HttpMessageParser::parseHeader(char ch, bool isTrailer)
{
	_headerFieldName.clear();
	_headerFieldValue.clear();
	if (Char::isCarriageReturn(ch)) {
		_state = isTrailer ? ParsingFinalLF : ParsingEndOfHeader;
	} else if (ch == ':') {
		setIsBad(ch, "Empty header field name");
	} else if (Http::isToken(ch)) {
		// Header field name is empty -> no length check
		_headerFieldName += ch;
		_state = isTrailer ? ParsingTrailerHeaderName : ParsingHeaderName;
	} else {
		std::ostringstream msg;
		msg << (isTrailer ? "Trailer's h" : "H") << "eader starts with invalid character";
		setIsBad(ch, msg.str());
	}
}

void HttpMessageParser::parseHeaderName(char ch, bool isTrailer)
{
	if (Char::isCarriageReturn(ch)) {
		setIsBad(ch, "Header is missing ':' separator");
	} else if (ch == ':') {
		_state = isTrailer ? ParsingTrailerHeaderValue : ParsingHeaderValue;
	} else if (Http::isToken(ch)) {
		if (_headerFieldName.length() < maxHeaderNameLength()) {
			_headerFieldName += ch;
		} else {
			setIsBad(ch, "Header name is too long");
		}
	} else {
		std::ostringstream msg;
		msg << (isTrailer ? "Trailer's h" : "H") << "eader name contains invalid character";
		setIsBad(ch, msg.str());
	}
}

void HttpMessageParser::parseHeaderValue(char ch, bool isTrailer)
{
	if (Char::isCarriageReturn(ch)) {
		_state = isTrailer ? ParsingTrailerHeaderValueLF : ParsingHeaderValueLF;
	} else if (!Http::isControl(ch)) {
		if (_headerFieldValue.length() < maxHeaderValueLength()) {
			_headerFieldValue += ch;
		} else {
			setIsBad(ch, "Header field is too long");
		}
	} else {
		std::ostringstream msg;
		msg << (isTrailer ? "Trailer's h" : "H") << "eader value contains invalid character";
		setIsBad(ch, msg.str());
	}
}

void HttpMessageParser::parseHeaderValueLF(char ch, bool isTrailer)
{
	if (Char::isLineFeed(ch)) {
		_state = isTrailer ? ParsingTrailerHeaderValueLWS : ParsingHeaderValueLWS;
	} else {
		std::ostringstream msg;
		msg << (isTrailer ? "Trailer's h" : "H") << "eader CR is followed by the invalid character";
		setIsBad(ch, msg.str());
	}
}

void HttpMessageParser::parseHeaderValueLWS(char ch, bool isTrailer)
{
	if (Char::isCarriageReturn(ch)) {
		appendHeader(ch);
		if (!isBad()) {
			_state = isTrailer ? ParsingFinalLF : ParsingEndOfHeader;
		}
	} else if (ch == ':') {
		setIsBad(ch, "Empty header name");
	} else if (Char::isSpaceOrTab(ch)) {
		if (_headerFieldValue.length() < maxHeaderValueLength()) {
			_headerFieldValue += ' ';
			_state = isTrailer ? ParsingTrailerHeaderValue : ParsingHeaderValue;
		} else {
			setIsBad(ch, "Header value is too long");
		}
	} else if (Http::isToken(ch)) {
		appendHeader(ch);
		if (!isBad()) {
			// Header field name is empty -> no length check
			_headerFieldName += ch;
			_state = isTrailer ? ParsingTrailerHeaderName : ParsingHeaderName;
		}
	} else {
		std::ostringstream msg;
		msg << (isTrailer ? "Trailer's h" : "H") << "eader starts with invalid character";
		setIsBad(ch, msg.str());
	}
}

//------------------------------------------------------------------------------
// HttpMessageParser::Error
//------------------------------------------------------------------------------

HttpMessageParser::Error::Error(SOURCE_LOCATION_ARGS_DECLARATION, char ch, int pos, int line, int col, const std::string& msg) :
	AbstractError(SOURCE_LOCATION_ARGS_PASSTHRU),
	_ch(ch),
	_pos(pos),
	_line(line),
	_col(col),
	_msg(msg)
{}

std::string HttpMessageParser::Error::composeMessage() const
{
	std::ostringstream oss;
	oss << "HTTP-message parsing error (pos: " << _pos << ", line: " << _line << ", col: " << _col <<
		", character: " << std::showbase << std::hex << static_cast<int>(_ch) << "): " << _msg;
	return oss.str();
}

} // namespace isl

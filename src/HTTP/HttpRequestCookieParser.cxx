#include <isl/HttpRequestCookieParser.hxx>

namespace isl
{

void HttpRequestCookieParser::reset()
{
	_parserState = ParsingCookie;
	_composerState = AwaitingVersion;
	_pos = 0;
	_curChar = 0;
	_isBad = false;
	_error.clear();
	_cookieName.clear(),
	_cookieValue.clear(),
	_cookieVersion.clear();
	_cookiePath.clear();
	_cookieDomain.clear();
	_cookiePort.clear();
	_currentAttrName.clear();
	_currentAttrValue.clear();
}

Http::RequestCookies HttpRequestCookieParser::parse(const std::string& headerValue)
{
	reset();
	Http::RequestCookies parsedCookies;
	Http::RequestCookie currentCookie;
	while (_pos < headerValue.length()) {
		_curChar = headerValue[_pos];
		switch (_parserState) {
			case ParsingCookie:
				if (Char::isSpaceOrTab(_curChar)) {
					// Nothing to do
				//} else if (Http::isToken(_curChar)) {					// This is correct (see RFC 2965) but browsers use utf-8 in cookie names
				} else if (!Http::isControl(_curChar) && (_curChar != '=')) {
					_currentAttrName.assign(1, _curChar);
					_parserState = ParsingAttribute;
				} else {
					std::wostringstream msg;
					msg << L"Cookie attribute has been started with a non-token character " << std::showbase << std::hex << static_cast<unsigned char>(_curChar) << L" at " << std::dec << _pos << L" position";
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
					setIsBad(msg.str());
				}
				break;
			case ParsingAttribute:
				if (_curChar == ';') {
					appendAttribute(parsedCookies, false);
					_parserState = ParsingCookie;
				} else if (_curChar == ',') {
					appendAttribute(parsedCookies, true);
					_parserState = ParsingCookie;
				} else if (Char::isSpaceOrTab(_curChar)) {
					_parserState = ParsingAttributeSP;
				} else if (_curChar == '=') {
					_parserState = ParsingEquals;
				//} else if (Http::isToken(_curChar)) {					// This is correct (see RFC 2965) but browsers use utf-8 in cookie names
				} else if (!Http::isControl(_curChar)) {
					_currentAttrName.append(1, _curChar);
				} else {
					std::wostringstream msg;
					msg << L"Cookie attribute contains an invalid character " << std::showbase << std::hex << static_cast<unsigned char>(_curChar) << L" at " << std::dec << _pos << L" position";
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
					setIsBad(msg.str());
				}
				break;
			case ParsingAttributeSP:
				if (_curChar == ';') {
					appendAttribute(parsedCookies, false);
					_parserState = ParsingCookie;
				} else if (_curChar == ',') {
					appendAttribute(parsedCookies, true);
					_parserState = ParsingCookie;
				} else if (_curChar == '=') {
					_parserState = ParsingEquals;
				} else if (Char::isSpaceOrTab(_curChar)) {
					// Nothing to do
				} else {
					std::wostringstream msg;
					msg << L"Cookie attribute trailing space is followed by an invalid character " << std::showbase << std::hex << static_cast<unsigned char>(_curChar) << L" at " << std::dec << _pos << L" position";
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
					setIsBad(msg.str());
				}
				break;
			case ParsingEquals:
				if (_curChar == ';') {
					appendAttribute(parsedCookies, false);
					_parserState = ParsingCookie;
				} else if (_curChar == ',') {
					appendAttribute(parsedCookies, true);
					_parserState = ParsingCookie;
				} else if (_curChar == '"') {
					_currentAttrValue.clear();
					_parserState = ParsingQuotedValue;
				} else if (Char::isSpaceOrTab(_curChar)) {
					// Nothing to do
				//} else if (Http::isToken(_curChar)) {				// This is correct (see RFC 2965) but browsers use unencoded separators in cookie values
				} else if (Http::isChar(_curChar) && !Http::isControl(_curChar)) {
					_currentAttrValue.assign(1, _curChar);
					_parserState = ParsingValue;
				} else {
					std::wostringstream msg;
					msg << L"Cookie attribute equals character is followed by an invalid character " << std::showbase << std::hex << static_cast<unsigned char>(_curChar) << L" at " << std::dec << _pos << L" position";
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
					setIsBad(msg.str());
				}
				break;
			case ParsingQuotedValue:
				if (_curChar == '"') {
					// Attribute/value pair has been parsed
					_parserState = ParsingValueSP;
				} else if (_curChar == '\\') {
					_parserState = ParsingQuotedValueBackslash;
				} else if (Http::isText(_curChar)) {
					_currentAttrValue.append(1, _curChar);
				} else {
					std::wostringstream msg;
					msg << L"Cookie attribute quoted value contains an invalid character " << std::showbase << std::hex << static_cast<unsigned char>(_curChar) << L" at " << std::dec << _pos << L" position";
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
					setIsBad(msg.str());
				}
				break;
			case ParsingQuotedValueBackslash:
				if (Http::isChar(_curChar)) {
					_currentAttrValue.append(1, _curChar);
					_parserState = ParsingQuotedValue;
				} else {
					std::wostringstream msg;
					msg << L"Cookie attribute quoted value quoted pair contains an invalid character " << std::showbase << std::hex << static_cast<unsigned char>(_curChar) << L" at " << std::dec << _pos << L" position";
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
					setIsBad(msg.str());
				}
				break;
			case ParsingValue:
				if (_curChar == ';') {
					appendAttribute(parsedCookies, false);
					_parserState = ParsingCookie;
				} else if (_curChar == ',') {
					appendAttribute(parsedCookies, true);
					_parserState = ParsingCookie;
				} else if (Char::isSpaceOrTab(_curChar)) {
					_parserState = ParsingValueSP;
				//} else if (Http::isToken(_curChar)) {				// This is correct (see RFC 2965) but browsers use unencoded separators in cookie values
				} else if (Http::isChar(_curChar) && !Http::isControl(_curChar)) {
					_currentAttrValue.append(1, _curChar);
				} else {
					std::wostringstream msg;
					msg << L"Cookie attribute value contains an invalid character " << std::showbase << std::hex << static_cast<unsigned char>(_curChar) << L" at " << std::dec << _pos << L" position";
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
					setIsBad(msg.str());
				}
				break;
			case ParsingValueSP:
				if (_curChar == ';') {
					appendAttribute(parsedCookies, false);
					_parserState = ParsingCookie;
				} else if (_curChar == ',') {
					appendAttribute(parsedCookies, true);
					_parserState = ParsingCookie;
				} else if (Char::isSpaceOrTab(_curChar)) {
					// Nothing to do
				} else {
					std::wostringstream msg;
					msg << L"Cookie attribute value is followed by invalid character " << std::showbase << std::hex << static_cast<unsigned char>(_curChar) << L" at " << std::dec << _pos << L" position";
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
					setIsBad(msg.str());
				}
				break;
			default:
				std::wostringstream msg;
				msg << L"Invalid HTTP-request cookie parse state: " << _parserState;
				setIsBad(msg.str());
				Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
				throw Exception(Error(SOURCE_LOCATION_ARGS, msg.str()));
		}
		if (_isBad) {
			break;
		}
		++_pos;
	}
	if (!_isBad) {
		if (_parserState == ParsingEquals || _parserState == ParsingValue || _parserState == ParsingValueSP) {
			// Appending the last cookie
			appendAttribute(parsedCookies, true);
		} else {
			const wchar_t msg[] = L"Premature end of request cookie header value";
			setIsBad(msg);
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg));
			throw Exception(Error(SOURCE_LOCATION_ARGS, msg));
		}
	}
	return parsedCookies;
}

void HttpRequestCookieParser::appendAttribute(Http::RequestCookies& parsedCookies, bool endOfCookieDetected)
{
	switch (_composerState) {
		case AwaitingVersion:
			if (_currentAttrName == "$Version") {
				_cookieVersion = _currentAttrValue;
				_composerState = AwaitingValue;
			} else if (_currentAttrName != "$Path" && _currentAttrName != "$Domain" && _currentAttrName != "$Port") {
				// Value received
				_cookieName = _currentAttrName;
				_cookieValue = _currentAttrValue;
				if (endOfCookieDetected) {
					appendCookie(parsedCookies);
				} else {
					_composerState = AwaitingPath;
				}
			} else {
				std::wostringstream msg;
				msg << L"Cookie version or value attribute expected instead of \"" << String::utf8Decode(_currentAttrName) << L"\" received";
				setIsBad(msg.str());
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
			break;
		case AwaitingValue:
			if (_currentAttrName != "$Path" && _currentAttrName != "$Domain" && _currentAttrName != "$Port") {
				// Value received
				_cookieName = _currentAttrName;
				_cookieValue = _currentAttrValue;
				if (endOfCookieDetected) {
					appendCookie(parsedCookies);
				} else {
					_composerState = AwaitingPath;
				}
			} else {
				std::wostringstream msg;
				msg << L"Cookie value attribute expected instead of \"" << String::utf8Decode(_currentAttrName) << L"\" received";
				setIsBad(msg.str());
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
			break;
		case AwaitingPath:
			if (_currentAttrName == "$Path") {
				_cookiePath = _currentAttrValue;
				_composerState = AwaitingDomain;
			} else if (_currentAttrName != "$Domain" && _currentAttrName != "$Port") {
				// Value received
				appendCookie(parsedCookies);			// Appending already composed cookie
				_cookieName = _currentAttrName;
				_cookieValue = _currentAttrValue;
				if (endOfCookieDetected) {
					appendCookie(parsedCookies);
				} else {
					_composerState = AwaitingPath;
				}
			} else {
				std::wostringstream msg;
				msg << L"Cookie path or value attribute expected instead of \"" << String::utf8Decode(_currentAttrName) << L"\" received";
				setIsBad(msg.str());
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
			break;
		case AwaitingDomain:
			if (_currentAttrName == "$Domain") {
				_cookieDomain = _currentAttrValue;
				_composerState = AwaitingPort;
			} else if (_currentAttrName != "$Path" && _currentAttrName != "$Port") {
				// Value received
				appendCookie(parsedCookies);			// Appending already composed cookie
				_cookieName = _currentAttrName;
				_cookieValue = _currentAttrValue;
				if (endOfCookieDetected) {
					appendCookie(parsedCookies);
				} else {
					_composerState = AwaitingPath;
				}
			} else {
				std::wostringstream msg;
				msg << L"Cookie domain or value attribute expected instead of \"" << String::utf8Decode(_currentAttrName) << L"\" received";
				setIsBad(msg.str());
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
			break;
		case AwaitingPort:
			if (_currentAttrName == "$Port") {
				_cookiePort = _currentAttrValue;
				appendCookie(parsedCookies);
			} else if (_currentAttrName != "$Path" && _currentAttrName != "$Domain") {
				// Value received
				appendCookie(parsedCookies);			// Appending already composed cookie
				_cookieName = _currentAttrName;
				_cookieValue = _currentAttrValue;
				if (endOfCookieDetected) {
					appendCookie(parsedCookies);
				} else {
					_composerState = AwaitingPath;
				}
			} else {
				std::wostringstream msg;
				msg << L"Cookie port or value attribute expected instead of \"" << String::utf8Decode(_currentAttrName) << L"\" received";
				setIsBad(msg.str());
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
			break;
		default:
			std::wostringstream msg;
			msg << L"Invalid HTTP-request cookie composer state: " << _composerState;
			setIsBad(msg.str());
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			throw Exception(Error(SOURCE_LOCATION_ARGS, msg.str()));
	}
	_currentAttrName.clear();
	_currentAttrValue.clear();
}

void HttpRequestCookieParser::appendCookie(Http::RequestCookies& parsedCookies)
{
	Http::RequestCookie cookie;
	cookie.name = _cookieName;
	cookie.value = String::decodePercent(_cookieValue);
	cookie.version = _cookieVersion;
	cookie.path = _cookiePath;
	cookie.domain = _cookieDomain;
	cookie.port = _cookiePort;
	parsedCookies.insert(Http::RequestCookies::value_type(cookie.name, cookie));
	_cookieName.clear(),
	_cookieValue.clear(),
	_cookiePath.clear();
	_cookieDomain.clear();
	_cookiePort.clear();
	_composerState = AwaitingValue;
}

} // namespace isl

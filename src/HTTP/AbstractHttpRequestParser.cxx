#include <isl/AbstractHttpRequestParser.hxx>
#include <isl/Http.hxx>
#include <isl/Exception.hxx>
#include <isl/HttpError.hxx>
#include <isl/String.hxx>
#include <isl/Utf8TextCodec.hxx>
#include <sstream>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractHttpRequestParser class
------------------------------------------------------------------------------*/

AbstractHttpRequestParser::AbstractHttpRequestParser() :
	_state(AbstractHttpRequestParser::ParsingRequest),
	_method(),
	_uri(),
	_version(),
	_headerFieldName(),
	_headerFieldValue(),
	_header(),
	_bodyChunk(),
	_pos(0),
	_line(1),
	_col(1),
	_maxMethodLength(20),
	_maxUriLength(4096),
	_maxVersionLength(20),
	_maxHeaderFieldNameLength(255),
	_maxHeaderFieldValueLength(4096)
{}

AbstractHttpRequestParser::~AbstractHttpRequestParser()
{}

void AbstractHttpRequestParser::reset()
{
	_state = ParsingRequest;
	_method.clear();
	_uri.clear();
	_version.clear();
	_headerFieldName.clear();
	_headerFieldValue.clear();
	_header.clear();
	_bodyChunk.clear();
	_pos = 0;
	_line = 1;
	_col = 1;
}

int AbstractHttpRequestParser::parse(const char * data, unsigned int size)
{
	if (isBadRequest() && isCompleteRequest()) {
		return 0;
	}
	for (unsigned int i = 0; i < size; ++i) {
		parse(data[i]);
		if (isCompleteRequest()) {
			return i;
		}
		++_pos;
		if (String::isLineFeed(data[i])) {
			++_line;
			_col = 1;
		}
	}
	return size - 1;
}

void AbstractHttpRequestParser::parse(char ch)
{
	switch (_state) {
	case ParsingRequest:
		if (isToken(ch)) {
			_method = ch;
			_state = ParsingMethod;
		} else {
			_state = BadRequest;
			std::wostringstream msg;
			msg << L"Request method starts with invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, msg.str());
			throw Exception(HttpError(HttpError::BadRequest(msg.str()), SOURCE_LOCATION_ARGS));
		}
		break;
	case ParsingMethod:
		if (isToken(ch)) {
			if (_method.length() < maxMethodLength()) {
				_method += ch;
			} else {
				_state = RequestMethodTooLong;
				Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, L"Request method is too long");
				//throw Exception(HttpError(HttpError::RequestMethodTooLong(), SOURCE_LOCATION_ARGS));
				throw Exception(HttpError(HttpError::BadRequest(L"Request method is too long"), SOURCE_LOCATION_ARGS));
			}
		} else if (isSpaceOrTab(ch)) {
			_state = ParsingMethodUriDelimeter;
			methodParsedPrivate();
		} else {
			_state = BadRequest;
			std::wostringstream msg;
			msg << L"Request method contains invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, msg.str());
			throw Exception(HttpError(HttpError::BadRequest(msg.str()), SOURCE_LOCATION_ARGS));
		}
		break;
	case ParsingMethodUriDelimeter:
		if (isSpaceOrTab(ch)) {
			// Just ignore it
		} else if (isAllowedInUri(ch)) {
			// TODO Use 'UriParser' class
			_state = ParsingUri;
			_uri = ch;
		} else {
			_state = BadRequest;
			std::wostringstream msg;
			msg << L"Request URI starts with invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, msg.str());
			throw Exception(HttpError(HttpError::BadRequest(msg.str()), SOURCE_LOCATION_ARGS));
		}
		break;
	case ParsingUri:
		if (isAllowedInUri(ch)) {
			if (_uri.length() < maxUriLength()) {
				// TODO Use 'UriParser' class
				_uri += ch;
			} else {
				_state = RequestUriTooLong;
				Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, L"Request URI is too long");
				throw Exception(HttpError(HttpError::RequestUriTooLong(), SOURCE_LOCATION_ARGS));
			}
		} else if (isSpaceOrTab(ch)) {
			_state = ParsingUriVersionDelimeter;
			uriParsedPrivate();
		} else {
			_state = BadRequest;
			std::wostringstream msg;
			msg << L"Request URI contains invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, msg.str());
			throw Exception(HttpError(HttpError::BadRequest(msg.str()), SOURCE_LOCATION_ARGS));
		}
		break;
	case ParsingUriVersionDelimeter:
		if (isSpaceOrTab(ch)) {
			// Just ignore it
		} else if (isAllowedInUri(ch)) {
			// TODO Use 'HttpVersionParser' class
			_state = ParsingVersion;
			_version = ch;
		} else {
			_state = BadRequest;
			std::wostringstream msg;
			msg << L"Request HTTP-version starts with invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, msg.str());
			throw Exception(HttpError(HttpError::BadRequest(msg.str()), SOURCE_LOCATION_ARGS));
		}
		break;
	case ParsingVersion:
		if (isSpaceOrTab(ch)) {
			_state = ParsingVersionCR;
			versionParsedPrivate();
		} else if (isCarriageReturn(ch)) {
			_state = ParsingVersionLF;
			versionParsedPrivate();
		} else if (isAllowedInVersion(ch)) {
			if (_version.length() < maxVersionLength()) {
				// TODO Use 'HttpVersionParser' class
				_version += ch;
			} else {
				_state = RequestVersionTooLong;
				Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, L"Request version is too long");
				//throw Exception(HttpError(HttpError::RequestVersionTooLong(), SOURCE_LOCATION_ARGS));
				throw Exception(HttpError(HttpError::BadRequest(L"Request version is too long"), SOURCE_LOCATION_ARGS));
			}
		} else {
			_state = BadRequest;
			std::wostringstream msg;
			msg << L"Request HTTP-version contains invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, msg.str());
			throw Exception(HttpError(HttpError::BadRequest(msg.str()), SOURCE_LOCATION_ARGS));
		}
		break;
	case ParsingVersionCR:
		if (isSpaceOrTab(ch)) {
			// Just ignore it
		} else if (isCarriageReturn(ch)) {
			_state = ParsingVersionLF;
		} else {
			_state = BadRequest;
			std::wostringstream msg;
			msg << L"Request HTTP-version is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, msg.str());
			throw Exception(HttpError(HttpError::BadRequest(msg.str()), SOURCE_LOCATION_ARGS));
		}
		break;
	case ParsingVersionLF:
		if (isLineFeed(ch)) {
			_state = ParsingHeaderField;
		} else {
			_state = BadRequest;
			std::wostringstream msg;
			msg << L"Request line's CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
			Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, msg.str());
			throw Exception(HttpError(HttpError::BadRequest(msg.str()), SOURCE_LOCATION_ARGS));
		}
		break;
	case ParsingHeaderField:
		_headerFieldName.clear();
		_headerFieldValue.clear();
		if (isCarriageReturn(ch)) {
			_state = ParsingEndOfHeader;
		} else if (ch == ':') {
			_state = ParsingHeaderFieldValue;
		} else if (isAllowedInHeader(ch)) {
			// TODO Use 'HeaderFieldParser' class
			_headerFieldName += ch;
			_state = ParsingHeaderFieldName;
		} else {
			_state = BadRequest;
			std::wostringstream msg;
			msg << L"Request header field starts with invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, msg.str());
			throw Exception(HttpError(HttpError::BadRequest(msg.str()), SOURCE_LOCATION_ARGS));
		}
		break;
	case ParsingHeaderFieldName:
		if (isCarriageReturn(ch)) {
			Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, L"Request header field is missing ':' separator");
			throw Exception(HttpError(HttpError::BadRequest(L"Request header field is missing ':' separator"), SOURCE_LOCATION_ARGS));
		} else if (ch == ':') {
			_state = ParsingHeaderFieldValue;
		} else if (isAllowedInHeader(ch)) {
			if (_headerFieldName.length() < maxHeaderFieldNameLength()) {
				// TODO Use 'HeaderFieldParser' class
				_headerFieldName += ch;
			} else {
				_state = RequestHeaderFieldNameTooLong;
				Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, L"Request header field name is too long");
				//throw Exception(HttpError(HttpError::RequestHeaderFieldNameTooLong(), SOURCE_LOCATION_ARGS));
				throw Exception(HttpError(HttpError::BadRequest(L"Request header field name is too long"), SOURCE_LOCATION_ARGS));
			}
		} else {
			_state = BadRequest;
			std::wostringstream msg;
			msg << L"Request header field name contains invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, msg.str());
			throw Exception(HttpError(HttpError::BadRequest(msg.str()), SOURCE_LOCATION_ARGS));
		}
		break;
	case ParsingHeaderFieldValue:
		if (isCarriageReturn(ch)) {
			_state = ParsingHeaderFieldValueLF;
		} else if (isAllowedInHeader(ch)) {
			if (_headerFieldValue.length() < maxHeaderFieldValueLength()) {
				// TODO Use 'HeaderFieldParser' class
				_headerFieldValue += ch;
			} else {
				_state = RequestHeaderFieldValueTooLong;
				Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, L"Request header field value is too long");
				throw Exception(HttpError(HttpError::BadRequest(L"Request header field value is too long"), SOURCE_LOCATION_ARGS));
			}
		} else {
			_state = BadRequest;
			std::wostringstream msg;
			msg << L"Request header field value contains invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, msg.str());
			throw Exception(HttpError(HttpError::BadRequest(msg.str()), SOURCE_LOCATION_ARGS));
		}
		break;
	case ParsingHeaderFieldValueLF:
		if (isLineFeed(ch)) {
			_state = ParsingHeaderFieldValueLWS;
		} else {
			_state = BadRequest;
			std::wostringstream msg;
			msg << L"Request header field's CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
			Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, msg.str());
			throw Exception(HttpError(HttpError::BadRequest(msg.str()), SOURCE_LOCATION_ARGS));
		}
		break;
	case ParsingHeaderFieldValueLWS:
		if (isCarriageReturn(ch)) {
			headerFieldParsedPrivate();
			_state = ParsingEndOfHeader;
		} else if (ch == ':') {
			headerFieldParsedPrivate();
			_headerFieldName.clear();
			_headerFieldValue.clear();
			_state = ParsingHeaderFieldValue;
		} else if (isSpaceOrTab(ch)) {
			_headerFieldValue += ' ';
			_state = ParsingHeaderFieldValue;
		} else if (isAllowedInHeader(ch)) {
			headerFieldParsedPrivate();
			_headerFieldName.clear();
			_headerFieldValue.clear();
			// TODO Use 'HeaderFieldParser' class
			_headerFieldName += ch;
			_state = ParsingHeaderFieldName;
		} else {
			_state = BadRequest;
			std::wostringstream msg;
			msg << L"Request header field starts with invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" at " << std::dec << _pos << L" position";
			Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, msg.str());
			throw Exception(HttpError(HttpError::BadRequest(msg.str()), SOURCE_LOCATION_ARGS));
		}
		break;
	case ParsingEndOfHeader:
		if (isLineFeed(ch)) {
			if (bodyExpected()) {
				_state = ParsingBody;
			} else {
				_state = ParsingCompleted;
			}
		} else {
			_state = BadRequest;
			std::wostringstream msg;
			msg << L"Request header's CR is followed by the invalid character " << std::showbase << std::hex <<
				static_cast<int>(ch) << L" instead of LF at " << std::dec << _pos << L" position";
			Http::debugLog.logDebug(SOURCE_LOCATION_ARGS, msg.str());
			throw Exception(HttpError(HttpError::BadRequest(msg.str()), SOURCE_LOCATION_ARGS));
		}
		break;
	default:
		_state = InvalidState;
		throw Exception(HttpError(HttpError::InvalidParserState(_state), SOURCE_LOCATION_ARGS));
	}
}

bool AbstractHttpRequestParser::needMoreData() const
{
	return (_state != ParsingCompleted);
}

AbstractHttpRequestParser::State AbstractHttpRequestParser::state() const
{
	return _state;
}

bool AbstractHttpRequestParser::isBadRequest() const
{
	return	_state == BadRequest ||
		_state == RequestUriTooLong ||
		_state == RequestVersionTooLong ||
		_state == RequestHeaderFieldNameTooLong ||
		_state == RequestHeaderFieldValueTooLong ||
		_state == RequestEntityTooLong ||
		_state == MethodNotImplemented ||
		_state == HTTPVersionNotImplemented ||
		_state == InvalidRequestURI;
}

bool AbstractHttpRequestParser::isCompleteRequest() const
{
	return (_state == ParsingCompleted);
}

bool AbstractHttpRequestParser::bodyExpected() const
{
	// TODO Working with HTTP-request with bodies
	return false;
}

void AbstractHttpRequestParser::methodParsedPrivate()
{
	methodParsed(_method);
}

void AbstractHttpRequestParser::uriParsedPrivate()
{
	// TODO Decode URI
	uriParsed(_uri);
}

void AbstractHttpRequestParser::versionParsedPrivate()
{
	versionParsed(_version);
}

void AbstractHttpRequestParser::headerFieldParsedPrivate()
{
	String::trim(_headerFieldName);
	String::trim(_headerFieldValue);
	_header.insert(Header::value_type(_headerFieldName, _headerFieldValue));
	headerFieldParsed(_headerFieldName, _headerFieldValue);
}

void AbstractHttpRequestParser::bodyChunkParsedPrivate()
{
}

//bool AbstractHttpRequestParser::parseURI()
//{
//	// TODO Handling full URI's, for example: http://www.host.com/dir/page.html?param=value#anchor
//	// Now we can work only with relative URI's, like /dir/page.html?param=value#anchor
//	if (_request->_uri[0] != '/') {
//		return false;
//	}
//	// Extracting requested resource
//	std::string::size_type pos = 0;
//	while ((pos < _request->_uri.length()) && (_request->_uri[pos] != '?') && (_request->_uri[pos] != '#')) {
//		_request->_resource += _request->_uri[pos];
//		++pos;
//	}
//	if ((pos >= _request->_uri.length()) || (_request->_uri[pos] == '#')) {
//		return true;
//	}
//	++pos;
//	// Extracting query string
//	while ((pos < _request->_uri.length()) && (_request->_uri[pos] != '#')) {
//		_request->_query += _request->_uri[pos];
//		++pos;
//	}
//	// Filling GET map
//	pos = 0;
//	while (pos < _request->_query.length()) {
//		std::string paramName;
//		std::string paramValue;
//		// Extracting param name
//		while ((pos < _request->_query.length()) && (_request->_query[pos] != '=')) {
//			paramName += _request->_query[pos];
//			++pos;
//		}
//		if (pos < _request->_query.length()) {
//			++pos;
//		}
//		// Extracting param value
//		while ((pos < _request->_query.length()) && (_request->_query[pos] != '&')) {
//			paramValue += _request->_query[pos];
//			++pos;
//		}
//		if (pos < _request->_query.length()) {
//			++pos;
//		}
//		if (!paramName.empty()) {
//			_request->_get[String::urlDecode(paramName)] = String::urlDecode(paramValue);
//		}
//	}
//	return true;
//}
//
//void AbstractHttpRequestParser::parseCookies()
//{
//	if (_headerFieldName != "Cookie") {
//		return;
//	}
//	int i = 0;
//	while (i < _headerFieldValue.length()) {						// I don't like this algorithm, but it works...
//		// Passing leading spaces
//		while ((i < _headerFieldValue.length()) && String::isSpaceOrTab(_headerFieldValue[i])) {
//			++i;
//		}
//		// Extracting cookie name
//		std::string cookieName;
//		while ((i < _headerFieldValue.length()) && (_headerFieldValue[i] != '=') && (_headerFieldValue[i] != ';')) {
//			cookieName += _headerFieldValue[i];
//			++i;
//		}
//		if (cookieName.empty()) {
//			// Finding next cookie;
//			while ((i < _headerFieldValue.length()) && (_headerFieldValue[i] != ';')) {
//				++i;
//			}
//			// Passing ';' if found
//			if (i < _headerFieldValue.length()) {
//				++i;
//			}
//			continue;
//		}
//		// Extracting cookie value
//		std::string cookieValue;
//		if ((i < _headerFieldValue.length()) && (_headerFieldValue[i] == '=')) {
//			++i;
//			while ((i < _headerFieldValue.length()) && (_headerFieldValue[i] != ';')) {
//				cookieValue += _headerFieldValue[i];
//				++i;
//			}
//		}
//		// Inserting cookie into HTTP-request
//		std::map<std::string, std::string>::const_iterator pos = _request->_cookies.find(cookieName);
//		if (pos == _request->_cookies.end()) {
//			_request->_cookies.insert(std::make_pair(cookieName, String::urlDecode(cookieValue)));
//			AbstractHTTPTask::debugLog.logMessage(L"Cookie \"" + Utf8TextCodec().decode(cookieName) + L"\"=\"" +
//					Utf8TextCodec().decode(String::urlDecode(cookieValue)) + L"\" added to request");
//		}
//		// Passing ';' if found
//		if (i < _headerFieldValue.length()) {
//			++i;
//		}
//	}
//}
//
//bool AbstractHttpRequestParser::canBeAddedToRequestURI(unsigned char ch) const
//{
//	return	(_request->_uri != "*") && (String::isAlpha(ch) || String::isDigit(ch) ||
//		(ch == '/') || (ch == '%') || (ch == '.') || (ch == '_') || (ch == '-') || (ch == '~') ||
//		(ch == '?') || (ch == '&') || (ch == '#')) || (ch == '=') /*|| (ch == '') || (ch == '') || (ch == '') || (ch == '')*/;
//}

} // namespace isl


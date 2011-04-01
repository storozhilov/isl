#include <isl/HTTPRequestParser.hxx>
#include <isl/HTTPRequest.hxx>
#include <isl/AbstractHTTPTask.hxx>
#include <isl/String.hxx>
#include <isl/Utf8TextCodec.hxx>
#include <sstream>
#include <stdexcept>

namespace isl
{

/*------------------------------------------------------------------------------
 * HTTPRequestParser class
------------------------------------------------------------------------------*/

HTTPRequestParser::HTTPRequestParser(HTTPRequest * request) :
	AbstractParser(),
	_request(request),
	_methods(),
	_methodsImplemented(),
	_versions(),
	_versionsImplemented(),
	_state(HTTPRequestParser::ParsingRequest),
	_requestMethodString(),
	_httpVersionString(),
	_headerFieldName(),
	_headerFieldValue()
{
	// HTTP-methods initialization
	_methods.push_back(new HTTPRequest::OPTIONS_HTTP_Method());
	_methods.push_back(new HTTPRequest::GET_HTTP_Method());
	_methods.push_back(new HTTPRequest::HEAD_HTTP_Method());
	_methods.push_back(new HTTPRequest::POST_HTTP_Method());
	_methods.push_back(new HTTPRequest::PUT_HTTP_Method());
	_methods.push_back(new HTTPRequest::DELETE_HTTP_Method());
	_methods.push_back(new HTTPRequest::TRACE_HTTP_Method());
	_methods.push_back(new HTTPRequest::CONNECT_HTTP_Method());
	// Implemented HTTP-methods initialization
	_methodsImplemented.push_back(new HTTPRequest::GET_HTTP_Method());
	// HTTP-versions initialization
	_versions.push_back(new HTTPRequest::HTTP_0_9_Version());
	_versions.push_back(new HTTPRequest::HTTP_1_0_Version());
	_versions.push_back(new HTTPRequest::HTTP_1_1_Version());
	// Implemented HTTP-versions initialization
	_versionsImplemented.push_back(new HTTPRequest::HTTP_1_0_Version());
	_versionsImplemented.push_back(new HTTPRequest::HTTP_1_1_Version());
	// Resetting HTTPRequest
	//_request.reset();
}

HTTPRequestParser::~HTTPRequestParser()
{
	for (TokenList::iterator i = _methods.begin(); i != _methods.end(); ++i) {
		delete (*i);
	}
	for (TokenList::iterator i = _methodsImplemented.begin(); i != _methodsImplemented.end(); ++i) {
		delete (*i);
	}
	for (TokenList::iterator i = _versions.begin(); i != _versions.end(); ++i) {
		delete (*i);
	}
	for (TokenList::iterator i = _versionsImplemented.begin(); i != _versionsImplemented.end(); ++i) {
		delete (*i);
	}
}

void HTTPRequestParser::reset()
{
	_state = ParsingRequest;
}

int HTTPRequestParser::parse(const char * data, unsigned int size)
{
	if (isBadRequest() && isCompleteRequest()) {
		return 0;
	}
	unsigned int pos = 0;
	while (pos < size) {
		switch (_state) {
		case ParsingRequest:
			_requestMethodString.clear();
			if (isFitTokens(_methods, _requestMethodString, data[pos])) {
				_requestMethodString += data[pos];
				++pos;
				_state = ParsingRequestMethod;
			} else {
				_state = BadRequest;
				std::wostringstream msg;
				msg << L"Request method starts with invalid character (code = " << static_cast<int>(data[pos]) << L')';
				AbstractHTTPTask::errorLog.logMessage(msg.str());		// TODO
				return pos;
			}
			break;
		case ParsingRequestMethod:
			// Extracting the _request.method
			while (pos < size) {
				if (String::isSpaceOrTab(data[pos])) {
					Token requestMethod = findToken(_methodsImplemented, _requestMethodString);
					if (requestMethod.isNull()) {
						_state = MethodNotImplemented;
						AbstractHTTPTask::errorLog.logMessage(L"Request method \"" +
								Utf8TextCodec().decode(_requestMethodString) + L"\" is not implemented");
						return pos;
					}
					_request->_method = requestMethod;
					_request->_uri.clear();
					_state = ParsingRequestURI;
					++pos;
					AbstractHTTPTask::debugLog.logMessage(L"Request method is \"" +
							Utf8TextCodec().decode(_requestMethodString) + L"\"");
					break;
				} else if (isFitTokens(_methods, _requestMethodString, data[pos])) {
					_requestMethodString += data[pos];
					++pos;
				} else {
					_state = BadRequest;
					std::wostringstream msg;
					msg << L"Request method contains invalid character (code = " << static_cast<int>(data[pos]) << L')';
					AbstractHTTPTask::errorLog.logMessage(msg.str());
					return pos;
				}
			}
			break;
		case ParsingRequestURI:
			// Passing leading spaces
			if (_request->_uri.empty()) {
				while (pos < size && String::isSpaceOrTab(data[pos])) {
					++pos;
				}
			}
			// Extracting Request-URI
			// TODO Checking Request-URI length
			while (pos < size) {
				if (String::isSpaceOrTab(data[pos])) {
					// TODO Check Request-URI (if needed)
					if (!parseURI()) {
						_state = InvalidRequestURI;
						AbstractHTTPTask::errorLog.logMessage(L"Invalid URI: \"" +
								Utf8TextCodec().decode(_request->_uri) + L"\"");
						return pos;
					}
					_httpVersionString.clear();
					_state = ParsingHTTPVersion;
					++pos;
					AbstractHTTPTask::debugLog.logMessage(L"Request-URI is \"" +
							Utf8TextCodec().decode(_request->_uri) + L"\"");
					break;
				} else if (canBeAddedToRequestURI(data[pos])) {
					_request->_uri += data[pos];
					++pos;
				} else {
					_state = BadRequest;
					std::wostringstream msg;
					msg << L"Request-URI contains invalid character (code = " << static_cast<int>(data[pos]) << L')';
					AbstractHTTPTask::errorLog.logMessage(msg.str());
					return pos;
				}
			}
			break;
		case ParsingHTTPVersion:
			// Skipping leading spaces
			if (_httpVersionString.empty()) {
				while (pos < size && String::isSpaceOrTab(data[pos])) {
					++pos;
				}
			}
			// Extracting HTTP-version
			while (pos < size) {
				if (String::isCarriageReturn(data[pos])) {
					Token httpVersion = findToken(_versionsImplemented, _httpVersionString);
					if (httpVersion.isNull()) {
						_state = HTTPVersionNotImplemented;
						AbstractHTTPTask::errorLog.logMessage(L"HTTP-version \"" +
								Utf8TextCodec().decode(_httpVersionString) + L"\" is not implemented");
						return pos;
					}
					_request->_version = httpVersion;
					_state = ParsingHTTPVersionCRLF;
					++pos;
					AbstractHTTPTask::debugLog.logMessage(L"HTTP-version is \"" +
							Utf8TextCodec().decode(_httpVersionString) + L"\"");
					break;
				} else if (isFitTokens(_versions, _httpVersionString, data[pos])) {
					_httpVersionString += data[pos];
					++pos;
				} else {
					_state = BadRequest;
					std::wostringstream msg;
					msg << L"HTTP-version contains invalid character (code = " << static_cast<int>(data[pos]) << L')';
					AbstractHTTPTask::errorLog.logMessage(msg.str());
					return pos;
				}
			}
			break;
		case ParsingHTTPVersionCRLF:
			if (String::isLineFeed(data[pos])) {
				_state = ParsingHeaderField;
				++pos;
			} else {
				_state = BadRequest;
				AbstractHTTPTask::errorLog.logMessage(L"Line feed after carriage return expected");
				return pos;
			}
			break;
		case ParsingHeaderField:
			if (String::isCarriageReturn(data[pos])) {
				_state = ParsingEndOfHeader;
				++pos;
			} else if (String::isToken(data[pos])) {
				_headerFieldName.clear();
				_headerFieldValue.clear();
				_state = ParsingHeaderFieldName;
				_headerFieldName += data[pos];
				++pos;
			} else {
				_state = BadRequest;
				std::wostringstream msg;
				msg << L"Header field name is starting with invalid character (code = " << static_cast<int>(data[pos]) << L')';
				AbstractHTTPTask::errorLog.logMessage(msg.str());
				return pos;
			}
			break;
		case ParsingHeaderFieldName:
			while (pos < size) {
				if (String::isSpaceOrTab(data[pos])) {
					_state = ParsingHeaderFieldSeparator;
					++pos;
					break;
				} else if (data[pos] == ':') {
					_state = ParsingHeaderFieldValue;
					++pos;
					AbstractHTTPTask::debugLog.logMessage(L"Request header field name is \"" +
							Utf8TextCodec().decode(_headerFieldName) + L"\"");
					break;
				} else if (String::isToken(data[pos])) {
					_headerFieldName += data[pos];
					++pos;
				} else {
					_state = BadRequest;
					std::wostringstream msg;
					msg << L"Header field name contains invalid character (code = " << static_cast<int>(data[pos]) << L')';
					AbstractHTTPTask::errorLog.logMessage(msg.str());
					return pos;
				}
			}
			break;
		case ParsingHeaderFieldSeparator:
			while (pos < size && String::isSpaceOrTab(data[pos])) {
				++pos;
			}
			if (pos < size) {
				if (data[pos] == ':') {
					_state = ParsingHeaderFieldValue;
					++pos;
					AbstractHTTPTask::debugLog.logMessage(L"Request header field name is \"" +
							Utf8TextCodec().decode(_headerFieldName) + L"\"");
					break;
				} else {
					_state = BadRequest;
					AbstractHTTPTask::errorLog.logMessage(L"Missing request header field separator ':'");
					return pos;
				}
			}
			break;
		case ParsingHeaderFieldValue:
			while (pos < size) {
				if (String::isCarriageReturn(data[pos])) {
					_state = ParsingHeaderFieldValueCRLF;
					++pos;
					break;
				} else if (!String::isControl(data[pos]) || String::isSeparator(data[pos])) {
					_headerFieldValue += data[pos];
					++pos;
				} else {
					_state = BadRequest;
					std::wostringstream msg;
					msg << L"Header field value contains invalid character (code = " << static_cast<int>(data[pos]) << L')';
					AbstractHTTPTask::errorLog.logMessage(msg.str());
					return pos;
				}
			}
			break;
		case ParsingHeaderFieldValueCRLF:
			if (String::isLineFeed(data[pos])) {
				_state = ParsingHeaderFieldValueLWS;
				++pos;
			} else {
				_state = BadRequest;
				AbstractHTTPTask::errorLog.logMessage(L"Line feed after carriage return expected");
				return pos;
			}
			break;
		case ParsingHeaderFieldValueLWS:
			if (String::isSpaceOrTab(data[pos])) {
				_headerFieldValue += "\r\n";
				_headerFieldValue += data[pos];
				_state = ParsingHeaderFieldValue;
				++pos;
			} else {
				AbstractHTTPTask::debugLog.logMessage(L"Request header field value is \"" +
						Utf8TextCodec().decode(_headerFieldValue) + L"\"");
				String::trim(_headerFieldName);
				String::trim(_headerFieldValue);
				parseCookies();
				_request->setHeaderField(_headerFieldName, _headerFieldValue, false);	// See last paragraph of 4.2 of RFC 2616
				_state = ParsingHeaderField;
			}
			break;
		case ParsingEndOfHeader:
			if (String::isLineFeed(data[pos])) {
				if (requestBodyExpected()) {
					_state = ParsingBody;
					++pos;
				} else {
					_state = ParsingCompleted;
					return pos + 1;							// Including character on current position
				}
			} else {
				_state = BadRequest;
				std::wostringstream msg;
				msg << L"Unexpected symbol after header leading CR (code = " << static_cast<int>(data[pos]) << L')';
				AbstractHTTPTask::errorLog.logMessage(msg.str());
			}
			break;
		case ParsingBody:
			// TODO
			AbstractHTTPTask::errorLog.logMessage(L"HTTP-requests with bodies are not implemented yet");
			return size;
			//break;
		default:
			// TODO
			throw std::runtime_error("Invalid HTTP-request parser state");
		}
	}
	// Full buffer parsed successfully
	return size;
}

bool HTTPRequestParser::needMoreData() const
{
	return (_state != ParsingCompleted);
}

HTTPRequestParser::State HTTPRequestParser::state() const
{
	return _state;
}

bool HTTPRequestParser::isBadRequest() const
{
	return	_state == BadRequest ||
		_state == RequestURITooLong ||
		_state == RequestHeaderTooLong ||
		_state == RequestEntityTooLong ||
		_state == MethodNotImplemented ||
		_state == HTTPVersionNotImplemented ||
		_state == InvalidRequestURI;
}

bool HTTPRequestParser::isCompleteRequest() const
{
	return (_state == ParsingCompleted);
}

bool HTTPRequestParser::requestBodyExpected() const
{
	// TODO Working with HTTP-request with bodies
	return false;
}

bool HTTPRequestParser::parseURI()
{
	// TODO Handling full URI's, for example: http://www.host.com/dir/page.html?param=value#anchor
	// Now we can work only with relative URI's, like /dir/page.html?param=value#anchor
	if (_request->_uri[0] != '/') {
		return false;
	}
	// Extracting requested resource
	std::string::size_type pos = 0;
	while ((pos < _request->_uri.length()) && (_request->_uri[pos] != '?') && (_request->_uri[pos] != '#')) {
		_request->_resource += _request->_uri[pos];
		++pos;
	}
	if ((pos >= _request->_uri.length()) || (_request->_uri[pos] == '#')) {
		return true;
	}
	++pos;
	// Extracting query string
	while ((pos < _request->_uri.length()) && (_request->_uri[pos] != '#')) {
		_request->_query += _request->_uri[pos];
		++pos;
	}
	// Filling GET map
	pos = 0;
	while (pos < _request->_query.length()) {
		std::string paramName;
		std::string paramValue;
		// Extracting param name
		while ((pos < _request->_query.length()) && (_request->_query[pos] != '=')) {
			paramName += _request->_query[pos];
			++pos;
		}
		if (pos < _request->_query.length()) {
			++pos;
		}
		// Extracting param value
		while ((pos < _request->_query.length()) && (_request->_query[pos] != '&')) {
			paramValue += _request->_query[pos];
			++pos;
		}
		if (pos < _request->_query.length()) {
			++pos;
		}
		if (!paramName.empty()) {
			_request->_get[String::urlDecode(paramName)] = String::urlDecode(paramValue);
		}
	}
	return true;
}

void HTTPRequestParser::parseCookies()
{
	if (_headerFieldName != "Cookie") {
		return;
	}
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
		std::map<std::string, std::string>::const_iterator pos = _request->_cookies.find(cookieName);
		if (pos == _request->_cookies.end()) {
			_request->_cookies.insert(std::make_pair(cookieName, String::urlDecode(cookieValue)));
			AbstractHTTPTask::debugLog.logMessage(L"Cookie \"" + Utf8TextCodec().decode(cookieName) + L"\"=\"" +
					Utf8TextCodec().decode(String::urlDecode(cookieValue)) + L"\" added to request");
		}
		// Passing ';' if found
		if (i < _headerFieldValue.length()) {
			++i;
		}
	}
}

bool HTTPRequestParser::canBeAddedToRequestURI(unsigned char ch) const
{
	return	(_request->_uri != "*") && (String::isAlpha(ch) || String::isDigit(ch) ||
		(ch == '/') || (ch == '%') || (ch == '.') || (ch == '_') || (ch == '-') || (ch == '~') ||
		(ch == '?') || (ch == '&') || (ch == '#')) || (ch == '=') /*|| (ch == '') || (ch == '') || (ch == '') || (ch == '')*/;
}

} // namespace isl


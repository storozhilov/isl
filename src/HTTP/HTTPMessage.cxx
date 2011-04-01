#include <isl/HTTPMessage.hxx>
#include <isl/String.hxx>
#include <isl/AbstractHTTPTask.hxx>
#include <sstream>

namespace isl
{

/*------------------------------------------------------------------------------
 * HTTPMessage
------------------------------------------------------------------------------*/

HTTPMessage::HTTPMessage(AbstractHTTPTask * task) :
	_task(task),
	_header(),
	_version(),
	_transferTimeout(DefaultTransferTimeout)						// TODO Use configuration subsystem
{}
	
HTTPMessage::~HTTPMessage()
{}

bool HTTPMessage::headerContains(const std::string &fieldName, const std::string &fieldValue) const
{
	std::string name(fieldName);
	std::string value(fieldValue);
	String::trim(name);
	String::trim(value);
	for (Header::const_iterator i = _header.begin(); i != _header.end(); ++i) {
		if (((*i).first == name) && ((*i).second == value)) {
			return true;
		}
	}
	return false;
}

std::string HTTPMessage::headerValue(const std::string &fieldName) const
{
	std::string name(fieldName);
	String::trim(name);
	std::string result;
	for (Header::const_iterator i = _header.begin(); i != _header.end(); ++i) {
		if ((*i).first == name) {
			if (result.empty()) {
				result = (*i).second;
			} else {
				result += (", " + (*i).second);				// See last paragraph of chapter 4.2, RFC 2616
			}
		}
	}
	return result;
}

std::list<std::string> HTTPMessage::headerValues(const std::string &fieldName) const
{
	std::string name(fieldName);
	String::trim(name);
	std::list<std::string> result;
	for (Header::const_iterator i = _header.begin(); i != _header.end(); ++i) {
		if ((*i).first == name) {
			result.push_back((*i).second);
		}
	}
	return result;
}

void HTTPMessage::setHeaderField(const std::string &fieldName, const std::string &fieldValue, bool replaceIfExists)
{
	std::string name(fieldName);
	std::string value(fieldValue);
	String::trim(name);
	String::trim(value);
	if (replaceIfExists) {
		_header.erase(name);
	}
	_header.insert(Header::value_type(name, value));
}

void HTTPMessage::setHeaderField(const std::string &fieldName, int fieldValue, bool replaceIfExists)
{
	std::ostringstream numStr;
	numStr << fieldValue;
	setHeaderField(fieldName, numStr.str(), replaceIfExists);
}

void HTTPMessage::resetHeaderField(const std::string &fieldName)
{
	std::string name(fieldName);
	String::trim(name);
	_header.erase(name);
}

void HTTPMessage::reset()
{
	_header.clear();
	_version.reset();
}

const AbstractParser::Token& HTTPMessage::version() const
{
	return _version;
}

} // namespace isl


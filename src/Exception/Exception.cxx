#include <isl/Exception.hxx>

namespace isl
{

Exception::Exception(const AbstractError& error) :
	_errors(),
	_message(),
	_debug(),
	_what()
{
	addError(error);
}

Exception::Exception(const Exception& rhs) :
	_errors(),
	_message(),
	_debug(),
	_what()
{
	for (ErrorList::const_iterator i = rhs._errors.begin(); i != rhs._errors.end(); ++i) {
		addError(**i);
	}
}

Exception::~Exception() throw()
{
	resetErrors();
}

Exception& Exception::operator=(const Exception& rhs)
{
	if (this == &rhs) {
		return *this;
	}
	resetErrors();
	for (ErrorList::const_iterator i = rhs._errors.begin(); i != rhs._errors.end(); ++i) {
		addError(**i);
	}
	return *this;
}

void Exception::addError(const AbstractError& error)
{
	if (!_errors.empty()) {
		_message += L'\n';
		_debug += L'\n';
		_what += '\n';
	}
	AbstractError * newError = error.clone();
	_errors.push_back(newError);
	_message += newError->message();
	_debug += newError->debug();
	_what += newError->what();
}

const wchar_t * Exception::message() const throw()
{
	return _message.c_str();
}

const wchar_t * Exception::debug() const throw()
{
	return _debug.c_str();
}

const char * Exception::what() const throw()
{
	return _what.c_str();
}

void Exception::resetErrors()
{
	for (ErrorList::iterator i = _errors.begin(); i != _errors.end(); ++i) {
		delete (*i);
	}
	_errors.clear();
}

} // namespace isl

#include <isl/Exception.hxx>
#include <isl/Utf8TextCodec.hxx>

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

Exception::Exception(const Exception& exception) :
	_errors(),
	_message(),
	_debug(),
	_what()
{
	for (ErrorList::const_iterator i = exception._errors.begin(); i != exception._errors.end(); ++i) {
		addError(*(*i));
	}
}

Exception::~Exception() throw()
{
	for (ErrorList::iterator i = _errors.begin(); i != _errors.end(); ++i) {
		delete (*i);
	}
}

void Exception::addError(const AbstractError& error)
{
	if (!_errors.empty()) {
		_message += L'\n';
		_debug += L'\n';
		_what += '\n';
	}
	_errors.push_back(error.clone());
	_message += error.message();
	_debug += error.debug();
	_what += Utf8TextCodec().encode(error.message());
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

} // namespace isl

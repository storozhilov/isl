#include <isl/AbstractError.hxx>
#include <isl/Utf8TextCodec.hxx>
#include <sstream>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractError
 *----------------------------------------------------------------------------*/

/*AbstractError::AbstractError(const AbstractType& type, SOURCE_LOCATION_ARGS_DECLARATION) :
	_typePtr(type.clone()),
	_message(type.message()),
	_debug(),
	_file(SOURCE_LOCATION_ARGS_FILE),
	_line(SOURCE_LOCATION_ARGS_LINE),
	_function(SOURCE_LOCATION_ARGS_FUNCTION)
{
	initDebug();
}

AbstractError::AbstractError(const std::wstring& message, SOURCE_LOCATION_ARGS_DECLARATION) :
	_typePtr(new DefaultType(message)),
	_message(message),
	_debug(),
	_file(SOURCE_LOCATION_ARGS_FILE),
	_line(SOURCE_LOCATION_ARGS_LINE),
	_function(SOURCE_LOCATION_ARGS_FUNCTION)
{
	initDebug();
}

AbstractError::AbstractError(const AbstractError& rhs) :
	_typePtr(rhs._typePtr->clone()),
	_message(rhs._message),
	_debug(rhs._debug),
	_file(rhs._file),
	_line(rhs._line),
	_function(rhs._function)
{}

AbstractError::~AbstractError()
{}

AbstractError& AbstractError::operator=(const AbstractError& rhs)
{
	_typePtr.reset(rhs._typePtr->clone());
	_message = rhs._message;
	_debug = rhs._debug;
	_file = rhs._file;
	_line = rhs._line;
	_function = rhs._function;
	return *this;
}

const AbstractError::AbstractType& AbstractError::type() const
{
	return *_typePtr;
}

const wchar_t * AbstractError::message() const throw()
{
	return _message.c_str();
}

const wchar_t * AbstractError::debug() const throw()
{
	return _debug.c_str();
}

const char * AbstractError::file() const throw()
{
	return _file.c_str();
}

unsigned int AbstractError::line() const throw()
{
	return _line;
}

const char * AbstractError::function() const throw()
{
	return _function.c_str();
}

void AbstractError::initDebug()
{
	std::wostringstream sstr;
	sstr << Utf8TextCodec().decode(_file) << L'(' << _line << L") " << Utf8TextCodec().decode(_function) << L": " << _message;
	_debug = sstr.str();
}*/

/*------------------------------------------------------------------------------
 * AbstractError_NEW
 *----------------------------------------------------------------------------*/

/*AbstractError_NEW::AbstractError_NEW(const AbstractType& type, SOURCE_LOCATION_ARGS_DECLARATION) :
	_typePtr(type.clone()),
	_message(type.message()),
	_debug(),
	_file(SOURCE_LOCATION_ARGS_FILE),
	_line(SOURCE_LOCATION_ARGS_LINE),
	_function(SOURCE_LOCATION_ARGS_FUNCTION)
{
	initDebug();
}

AbstractError_NEW::AbstractError_NEW(const std::wstring& message, SOURCE_LOCATION_ARGS_DECLARATION) :
	_typePtr(new DefaultType(message)),
	_message(message),
	_debug(),
	_file(SOURCE_LOCATION_ARGS_FILE),
	_line(SOURCE_LOCATION_ARGS_LINE),
	_function(SOURCE_LOCATION_ARGS_FUNCTION)
{
	initDebug();
}

AbstractError_NEW::AbstractError_NEW(const AbstractError_NEW& rhs) :
	_typePtr(rhs._typePtr->clone()),
	_message(rhs._message),
	_debug(rhs._debug),
	_file(rhs._file),
	_line(rhs._line),
	_function(rhs._function)
{}

AbstractError_NEW::~AbstractError_NEW()
{}

AbstractError_NEW& AbstractError_NEW::operator=(const AbstractError_NEW& rhs)
{
	_typePtr.reset(rhs._typePtr->clone());
	_message = rhs._message;
	_debug = rhs._debug;
	_file = rhs._file;
	_line = rhs._line;
	_function = rhs._function;
	return *this;
}

const AbstractError_NEW::AbstractType& AbstractError_NEW::type() const
{
	return *_typePtr;
}

const wchar_t * AbstractError_NEW::message() const throw()
{
	return _message.c_str();
}

const wchar_t * AbstractError_NEW::debug() const throw()
{
	return _debug.c_str();
}

const char * AbstractError_NEW::file() const throw()
{
	return _file.c_str();
}

unsigned int AbstractError_NEW::line() const throw()
{
	return _line;
}

const char * AbstractError_NEW::function() const throw()
{
	return _function.c_str();
}

void AbstractError_NEW::initDebug()
{
	std::wostringstream sstr;
	sstr << Utf8TextCodec().decode(_file) << L'(' << _line << L") " << Utf8TextCodec().decode(_function) << L": " << _message;
	_debug = sstr.str();
}*/

} // namespace isl


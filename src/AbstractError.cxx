#include <isl/AbstractError.hxx>

namespace isl
{

AbstractError::AbstractError(SOURCE_LOCATION_ARGS_DECLARATION) :
	_file(SOURCE_LOCATION_ARGS_FILE),
	_line(SOURCE_LOCATION_ARGS_LINE),
	_function(SOURCE_LOCATION_ARGS_FUNCTION),
	_isComposed(false),
	_message()
{}

AbstractError::~AbstractError()
{}

const std::string& AbstractError::message() const
{
	if (!_isComposed) {
		_message = composeMessage();
		_isComposed = true;
	}
	return _message;
}

std::string AbstractError::sourceLocation() const
{
	return composeSourceLocation(file(), _line, function());
}

} // namespace isl

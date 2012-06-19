#include <isl/AbstractLogMessage.hxx>
#include <sstream>

namespace isl
{

AbstractLogMessage::AbstractLogMessage(SOURCE_LOCATION_ARGS_DECLARATION) :
	_timestamp(DateTime::now()),
	_file(SOURCE_LOCATION_ARGS_FILE),
	_line(SOURCE_LOCATION_ARGS_LINE),
	_function(SOURCE_LOCATION_ARGS_FUNCTION),
	_message(),
	_isComposed(false)
{}

AbstractLogMessage::~AbstractLogMessage()
{}

const std::string& AbstractLogMessage::message() const
{
	if (!_isComposed) {
		_message = compose();
		_isComposed = true;
	}
	return _message;
}

std::string AbstractLogMessage::sourceLocation() const
{
	return composeSourceLocation(file(), _line, function());
}

} // namespace isl

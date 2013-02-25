#include <isl/AbstractLogMessage.hxx>
#include <sstream>

namespace isl
{

AbstractLogMessage::AbstractLogMessage(SOURCE_LOCATION_ARGS_DECLARATION) :
	_timestamp(Timestamp::now()),
	_file(SOURCE_LOCATION_ARGS_FILE),
	_line(SOURCE_LOCATION_ARGS_LINE),
	_function(SOURCE_LOCATION_ARGS_FUNCTION),
	_message(),
	_isMessageComposed(false),
	_location(),
	_isLocationComposed(false)
{}

AbstractLogMessage::~AbstractLogMessage()
{}

const std::string& AbstractLogMessage::message() const
{
	if (!_isMessageComposed) {
		_message = compose();
		_isMessageComposed = true;
	}
	return _message;
}

const std::string& AbstractLogMessage::location() const
{
	if (!_isLocationComposed) {
		_location = composeSourceLocation(_file.c_str(), _line, _function.c_str());
		_isLocationComposed = true;
	}
	return _location;
}

} // namespace isl

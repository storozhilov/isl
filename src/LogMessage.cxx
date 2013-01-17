#include <isl/LogMessage.hxx>

namespace isl
{

LogMessage::LogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::string& msg) :
	AbstractLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
	_msg()
{
	_msg << msg;
}

std::string LogMessage::compose() const
{
	return _msg.str();
}

} // namespace isl

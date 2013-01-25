#include <isl/LogMessage.hxx>

namespace isl
{

LogMessage::LogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::string& msg) :
	AbstractLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
	_msg()
{
	_msg << msg;
}

LogMessage::LogMessage(const LogMessage& rhs) :
	AbstractLogMessage(rhs),
	_msg()
{
	_msg << rhs._msg.rdbuf();
}

LogMessage& LogMessage::operator=(const LogMessage& rhs)
{
	if (&rhs != this) {
		_msg.str("");
		_msg << rhs._msg.rdbuf();
	}
	return *this;
}

std::string LogMessage::compose() const
{
	return _msg.str();
}

} // namespace isl

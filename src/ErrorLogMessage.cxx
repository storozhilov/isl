#include <isl/ErrorLogMessage.hxx>
#include <sstream>

namespace isl
{

ErrorLogMessage::ErrorLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const AbstractError& err) :
	AbstractLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
	_msg(composeStatic(err))
{}

std::string ErrorLogMessage::compose() const
{
	return _msg;
}

std::string ErrorLogMessage::composeStatic(const AbstractError& err)
{
	std::ostringstream oss;
	oss << "Error detected at " << err.sourceLocation() << ": " << err.message();
	if (!err.info().empty()) {
		oss << ". Info: " << err.info();
	}
	oss << '.';
	return oss.str();
}

} // namespace isl

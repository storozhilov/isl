#include <isl/ExceptionLogMessage.hxx>
#include <isl/Exception.hxx>
#include <sstream>

namespace isl
{

ExceptionLogMessage::ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt, const std::string& contextInfo) :
	AbstractLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
	_msg(composeStatic(expt, contextInfo))
{}

std::string ExceptionLogMessage::compose() const
{
	return _msg;
}

std::string ExceptionLogMessage::composeStatic(const std::exception& expt, const std::string& contextInfo)
{
	std::ostringstream oss;
	const Exception * e = dynamic_cast<const Exception *>(&expt);
	if (e) {
		oss << "Exception detected at " << e->error().sourceLocation() << ": ";
	} else {
		oss << "Exception detected: ";
	}
	oss << expt.what();
	if (e && !e->error().info().empty()) {
		oss << ". Exception info: " << e->error().info();
	}
	if (!contextInfo.empty()) {
		oss << ". Context info: " << contextInfo;
	}
	oss << '.';
	return oss.str();
}

} // namespace isl

#include <isl/ExceptionLogMessage.hxx>
#include <isl/Exception.hxx>
#include <sstream>

namespace isl
{

ExceptionLogMessage::ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt) :
	AbstractLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
	_expt(expt),
	_info()
{}

ExceptionLogMessage::ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt, const std::string& info) :
	AbstractLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
	_expt(expt),
	_info(info)
{}

std::string ExceptionLogMessage::compose() const
{
	std::ostringstream oss;
	const Exception * e = dynamic_cast<const Exception *>(&_expt);
	if (e) {
		oss << "isl::Exception detected in " << e->error().sourceLocation() << ": ";
	} else {
		oss << "std::exception detected: ";
	}
	oss << _expt.what();
	if (!_info.empty()) {
		oss << ". Info: " << _info;
	}
	return oss.str();
}

} // namespace isl

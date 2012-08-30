#ifndef ISL__EXCEPTION_LOG_MESSAGE__HXX
#define ISL__EXCEPTION_LOG_MESSAGE__HXX

#include <isl/AbstractLogMessage.hxx>

namespace isl
{

//! Log message about exception
class ExceptionLogMessage : public AbstractLogMessage
{
public:
	ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt, const std::string& contextInfo = std::string());
private:
	ExceptionLogMessage();

	//! Composes and returns exception log message
	virtual std::string compose() const;

	static std::string composeStatic(const std::exception& expt, const std::string& contextInfo);

	const std::string _msg;
};

} // namespace isl

#endif

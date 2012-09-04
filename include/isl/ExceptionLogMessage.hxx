#ifndef ISL__EXCEPTION_LOG_MESSAGE__HXX
#define ISL__EXCEPTION_LOG_MESSAGE__HXX

#include <isl/AbstractLogMessage.hxx>

namespace isl
{

//! Log message about exception
class ExceptionLogMessage : public AbstractLogMessage
{
public:
	//! Constructs exception log message
	/*!
	  You should use SOURCE_LOCATION_ARGS macro as a value for the first parameter

	  \param expt Constant reference to the exception object
	  \param contextInfo User info
	*/
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

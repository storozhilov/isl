#ifndef ISL__EXCEPTION_LOG_MESSAGE__HXX
#define ISL__EXCEPTION_LOG_MESSAGE__HXX

#include <isl/AbstractLogMessage.hxx>

namespace isl
{

//! Log message abstract class
class ExceptionLogMessage : public AbstractLogMessage
{
public:
	ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt);
	ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt, const std::string& info);
private:
	ExceptionLogMessage();

	//! Composes and returns exception log message
	virtual std::string compose() const;

	const std::exception& _expt;
	const std::string _info;
};

} // namespace isl

#endif

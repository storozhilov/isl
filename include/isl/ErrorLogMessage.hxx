#ifndef ISL__ERROR_LOG_MESSAGE__HXX
#define ISL__ERROR_LOG_MESSAGE__HXX

#include <isl/AbstractLogMessage.hxx>
#include <isl/AbstractError.hxx>

namespace isl
{

//! Log message about error
class ErrorLogMessage : public AbstractLogMessage
{
public:
	ErrorLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const AbstractError& err);
private:
	ErrorLogMessage();

	//! Composes and returns exception log message
	virtual std::string compose() const;

	static std::string composeStatic(const AbstractError& err);

	const std::string _msg;
};

} // namespace isl

#endif

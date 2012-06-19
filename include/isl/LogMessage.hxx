#ifndef ISL__LOG_MESSAGE__HXX
#define ISL__LOG_MESSAGE__HXX

#include <isl/AbstractLogMessage.hxx>

namespace isl
{

//! Basic log message
class LogMessage : public AbstractLogMessage
{
public:
	LogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::string& msg);
private:
	LogMessage();
	
	//! Composes and returns notification log message
	virtual std::string compose() const;

	const std::string _msg;
};

} // namespace isl

#endif

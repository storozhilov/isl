#ifndef ISL__LOG_MESSAGE__HXX
#define ISL__LOG_MESSAGE__HXX

#include <isl/AbstractLogMessage.hxx>
#include <sstream>

namespace isl
{

//! Basic log message
class LogMessage : public AbstractLogMessage
{
public:
	//! Constructs log message
	/*!
	  \param SOURCE_LOCATION_ARGS_DECLARATION Put SOURCE_LOCATION_ARGS macro here
	  \param msg Message text
	*/
	LogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::string& msg = std::string());
	//! Copying constructor
	/*!
	  \param rhs Log message to copy from
	*/
	LogMessage(const LogMessage& rhs);
	//! Assignment operator
	/*!
	  \param rhs Log message to assign from
	  \return reference to log message object
	*/
	LogMessage& operator=(const LogMessage& rhs);
	//! Stream output operator templated redefinition
	/*!
	  \param val Value to append to message text internal string stream
	  \return Reference to log message object
	  \tparam T Value type
	*/
	template <typename T> LogMessage& operator<<(const T& val)
	{
		_msg << val;
		return *this;
	}
private:
	LogMessage();
	
	//! Composes and returns notification log message
	virtual std::string compose() const;

	std::ostringstream _msg;
};

} // namespace isl

#endif

#ifndef ISL__ABSTRACT_LOG_MESSAGE__HXX
#define ISL__ABSTRACT_LOG_MESSAGE__HXX

#include <isl/Debug.hxx>
#include <isl/DateTime.hxx>

namespace isl
{

//! Log message abstract class
class AbstractLogMessage
{
public:
	//! Constructs log message
	/*!
	  You should use SOURCE_LOCATION_ARGS macro as a value for the first parameter
	*/
	AbstractLogMessage(SOURCE_LOCATION_ARGS_DECLARATION);
	//! Destructor
	virtual ~AbstractLogMessage();

	//! Returns log message timestamp
	inline const DateTime& timestamp() const
	{
		return _timestamp;
	}
	//! Returns source code filename where the message was created
	inline const char * file() const throw()
	{
		return _file.c_str();
	}
	//! Returns source code line where the message was created
	inline unsigned int line() const throw()
	{
		return _line;
	}
	//! Returns source code function where the message was created
	inline const char * function() const throw()
	{
		return _function.c_str();
	}
	//! Returns log message
	const std::string& message() const;
	//! Composing string with source code filename, line number and function name help function
	std::string sourceLocation() const;
protected:
	//! Composes & returns log message
	virtual std::string compose() const = 0;
private:
	AbstractLogMessage();

	const DateTime _timestamp;
	const std::string _file;
	const unsigned int _line;
	const std::string _function;
	mutable std::string _message;
	mutable bool _isComposed;
};

} // namespace isl

#endif

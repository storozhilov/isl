#ifndef ISL__ABSTRACT_LOG_MESSAGE__HXX
#define ISL__ABSTRACT_LOG_MESSAGE__HXX

#include <isl/Debug.hxx>
#include <isl/Timestamp.hxx>

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
	inline const Timestamp& timestamp() const
	{
		return _timestamp;
	}
	//! Returns source code filename where the message was created
	inline const std::string& file() const
	{
		return _file;
	}
	//! Returns source code line where the message was created
	inline unsigned int line() const
	{
		return _line;
	}
	//! Returns source code function where the message was created
	inline const std::string& function() const
	{
		return _function;
	}
	//! Returns log message
	const std::string& message() const;
	//! Returns log message source location text representation
	const std::string& location() const;
protected:
	//! Composes & returns log message
	virtual std::string compose() const = 0;
private:
	AbstractLogMessage();

	const Timestamp _timestamp;
	const std::string _file;
	const unsigned int _line;
	const std::string _function;
	mutable std::string _message;
	mutable bool _isMessageComposed;
	mutable std::string _location;
	mutable bool _isLocationComposed;
};

} // namespace isl

#endif

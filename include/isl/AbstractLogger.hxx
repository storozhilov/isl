#ifndef ISL__ABSTRACT_LOGGER__HXX
#define ISL__ABSTRACT_LOGGER__HXX

#include <isl/AbstractLogMessage.hxx>

namespace isl
{

class AbstractLogTarget;
class Log;

//! Logging engine abstract class
class AbstractLogger
{
public:
	//! Constructor
	AbstractLogger()
	{}
	//! Destructor
	virtual ~AbstractLogger()
	{}
protected:
	//! Registers a log target
	/*!
	  \param target Reference to target to register
	  /note Thread-unsafe
	*/
	virtual void registerTarget(AbstractLogTarget& target) = 0;
	//! Unregisters a log target
	/*!
	  \param target Reference to target to unregister
	  /note Thread-unsafe
	*/
	virtual void unregisterTarget(AbstractLogTarget& target) = 0;
	//! Writes log message to target
	/*!
	  \param target Reference to target to write message to
	  \param msg Reference to message to write
	  \param prefix Log message prefix
	  /note Thread-safe
	*/
	virtual void log(AbstractLogTarget& target, const AbstractLogMessage& msg, const std::string& prefix) = 0;
private:
	friend class AbstractLogTarget;
	friend class Log;
};

} // namespace isl

#endif

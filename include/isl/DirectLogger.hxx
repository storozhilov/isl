#ifndef ISL__DIRECT_LOGGER__HXX
#define ISL__DIRECT_LOGGER__HXX

#include <isl/AbstractLogger.hxx>
#include <isl/Mutex.hxx>
#include <map>

namespace isl
{

//! Direct logging engine class
/*!
  Log message is to be written to target in the same thread for the price of one mutex per log target.
  Use this engine for software debugging, cause it does not start any additional thread for deferred logging.
*/
class DirectLogger : public AbstractLogger
{
public:
	//! Constructor
	DirectLogger();
	//! Destructor
	~DirectLogger();
private:
	typedef std::map<AbstractLogTarget *, Mutex *> TargetsContainer;

	//! Registers a log target
	/*!
	  \param target Reference to target to register
	  /note Thread-unsafe
	*/
	virtual void registerTarget(AbstractLogTarget& target);
	//! Unregisters a log target
	/*!
	  \param target Reference to target to unregister
	  /note Thread-unsafe
	*/
	virtual void unregisterTarget(AbstractLogTarget& target);
	//! Writes log message to target
	/*!
	  \param target Reference to target to write message to
	  \param msg Reference to message to write
	  \param prefix Log message prefix
	  /note Thread-safe
	*/
	virtual void log(AbstractLogTarget& target, const AbstractLogMessage& msg, const std::string& prefix);

	TargetsContainer _targets;
};

} // namespace isl

#endif

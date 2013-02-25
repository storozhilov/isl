#ifndef ISL__ABSTRACT_LOG_TARGET__HXX
#define ISL__ABSTRACT_LOG_TARGET__HXX

#include <isl/AbstractLogger.hxx>
#include <set>

namespace isl
{

//! Log target abstract class
class AbstractLogTarget
{
public:
	//! Constructs log target
	/*!
	  \param logger Regerence to logging engine
	*/
	AbstractLogTarget(AbstractLogger& logger);
	//! Destructor
	virtual ~AbstractLogTarget();
	//! Returns reference to logging engine
	inline AbstractLogger& logger()
	{
		return _logger;
	}
	//! Writes a log message
	/*!
	  \param msg Log message to write
	  \param prefix Log message prefix
	  \note Thread-unsafe
	*/
	virtual void log(const AbstractLogMessage& msg, const std::string& prefix) = 0;
private:
	typedef std::set<Log *> LogsContainer;

	AbstractLogTarget();
	AbstractLogTarget(AbstractLogTarget&);					// No copy

	AbstractLogTarget& operator=(AbstractLogTarget&);			// No copy

	AbstractLogger& _logger;
	LogsContainer _logs;

	friend class Log;
};

} // namespace isl

#endif


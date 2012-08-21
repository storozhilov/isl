#ifndef ISL__ABSTRACT_LOG_DEVICE__HXX
#define ISL__ABSTRACT_LOG_DEVICE__HXX

#include <isl/Mutex.hxx>
#include <string>

namespace isl
{

class Log;
class AbstractLogMessage;
class AbstractLogTarget;

//! Logging device abstract base class
/*!
  One loging device could serve multiple logging targets, e.g. one file and a symlink to this file are serving by
  the same logging device which represents it's file.

  \sa AbstractLogTarget
*/
class AbstractLogDevice
{
public:
	AbstractLogDevice();
	virtual ~AbstractLogDevice();
	//! Thread safely writes log message to the log device
	/*!
	  \param log Constant reference to log object
	  \param msg Constant reference to log message object to write to the log device
	*/
	void logMessage(const Log& log, const AbstractLogMessage& msg);
	//! Returns true if the log device is serving passed log target
	/*!
	  \param target Pointer to the log target to check if the device is serving it or not.
	*/
	//virtual bool serving(const AbstractLogTarget * target) const = 0;
protected:
	//! Returns true if the log device is serving passed log target
	/*!
	  \param target Pointer to the log target to check if the device is serving it or not.
	*/
	virtual bool serving(const AbstractLogTarget * target) const = 0;
	//! Thread unsafely writing log message to the log device abstract virtual method
	/*!
	  \param log Constant reference to log object
	  \param msg Constant reference to log message object to write to the log device
	*/
	virtual void writeMessage(const Log& log, const AbstractLogMessage& msg) = 0;
private:
	Mutex _writeMutex;

	friend class LogDispatcher;
};

} // namespace isl

#endif


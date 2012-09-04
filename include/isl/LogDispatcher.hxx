#ifndef ISL__LOG_DISPATCHER__HXX
#define ISL__LOG_DISPATCHER__HXX

#include <isl/ReadWriteLock.hxx>
#include <string>
#include <list>
#include <set>

#include <vector>
#include <map>

namespace isl
{

class Log;
class AbstractLogMessage;
class AbstractLogTarget;
class AbstractLogDevice;

//! Logging dispatcher
/*!
  This is internal ISL class not for any usage.
*/
class LogDispatcher
{
public:
	//! Constructs log dispatcher
	LogDispatcher();
	//! Destructor
	~LogDispatcher();

	//! Connects log to the target's device
	/*!
	  \param log Pointer to log
	  \param target Pointer to target
	*/
	void connectLogToDevice(Log * log, const AbstractLogTarget * target);
	//! Disconnects log from the target's device
	/*!
	  \param log Pointer to log
	  \param target Pointer to target
	*/
	void disconnectLogFromDevice(Log * log, const AbstractLogTarget * target);
	//! Disconnects log from all devices
	/*!
	  \param log Pointer to log
	*/
	void disconnectLogFromDevices(Log * log);
	//! Writes message to the devices of all connected targets
	/*!
	  \param log Pointer to log
	  \param msg Constant reference to message to write
	*/
	void logMessage(Log * log, const AbstractLogMessage& msg);
private:
	LogDispatcher(const LogDispatcher&);

	LogDispatcher& operator=(const LogDispatcher&);

	typedef std::set<AbstractLogDevice *> Devices;
	typedef std::multimap<Log *, Devices::iterator> Connections;

	void sweepDevices();

	Devices _devices;
	Connections _connections;
	ReadWriteLock _connectionsRWLock;
};

} // namespace isl

#endif


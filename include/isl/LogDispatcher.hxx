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
class AbstractLogTarget;
class AbstractLogDevice;

class LogDispatcher
{
public:
	LogDispatcher();
	~LogDispatcher();

	void connectLogToDevice(Log *log, const AbstractLogTarget *target);
	void disconnectLogFromDevice(Log *log, const AbstractLogTarget *target);
	void disconnectLogFromDevices(Log *log);
	void logMessage(Log *log, const std::wstring &msg);
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


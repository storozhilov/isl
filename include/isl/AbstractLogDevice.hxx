#ifndef ISL__ABSTRACT_LOG_DEVICE__HXX
#define ISL__ABSTRACT_LOG_DEVICE__HXX

#include <isl/Mutex.hxx>
#include <string>

namespace isl
{

class Log;
class AbstractLogMessage;
class AbstractLogTarget;

class AbstractLogDevice
{
public:
	AbstractLogDevice();
	virtual ~AbstractLogDevice();

	void logMessage(const Log& log, const AbstractLogMessage& msg);

	virtual bool serving(const AbstractLogTarget* target) const = 0;
protected:
	virtual void writeMessage(const Log& log, const AbstractLogMessage& msg) = 0;
private:
	Mutex _writeMutex;
};

} // namespace isl

#endif


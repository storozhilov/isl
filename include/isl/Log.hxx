#ifndef ISL__LOG__HXX
#define ISL__LOG__HXX

#include <isl/LogDispatcher.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ReadWriteLock.hxx>
#include <isl/Exception.hxx>
#include <stdexcept>
#include <string>
#include <sstream>
#include <ostream>

namespace isl
{

class AbstractLogTarget;

//! Thread-safe logging implementation class
class Log
{
public:
	Log();
	Log(const std::wstring& prefix);
	virtual ~Log();

	//! Connects target to log
	void connectTarget(const AbstractLogTarget& target);
	//! Disconnects target from the log
	void disconnectTarget(const AbstractLogTarget& target);
	//! Disconnects all targets from the log
	void disconnectTargets();
	//! Logs message
	void log(const AbstractLogMessage& msg);
	//! Logs message
	void log(const std::string& msg);
	//! Logs message
	void log(const std::wstring& msg);
	//! Sets log prefix
	void setPrefix(const std::wstring& newPrefix);
	//! Returns log prefix
	std::wstring prefix() const;
private:
	Log(const Log&);							// No copy

	Log& operator=(const Log&);						// No copy
	
	std::wstring _prefix;
	mutable ReadWriteLock _logRWLock;

	static std::wstring composeSourceLocation(SOURCE_LOCATION_ARGS_DECLARATION);

	static LogDispatcher dispatcher;
};

} // namespace isl

#endif


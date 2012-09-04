#ifndef ISL__LOG__HXX
#define ISL__LOG__HXX

#include <isl/LogDispatcher.hxx>
#include <isl/AbstractLogMessage.hxx>
#include <isl/Exception.hxx>
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
	//! Default constructor
	Log();
	//! Constructs log
	/*!
	  \param prefix Log prefix wich is to be printed to log target
	  \param composeSourceLocation Print source location to log target if TRUE
	*/
	Log(const std::string& prefix, bool composeSourceLocation = true);
	//! Destructor
	virtual ~Log();

	//! Connects target to log
	void connectTarget(const AbstractLogTarget& target);
	//! Disconnects target from the log
	void disconnectTarget(const AbstractLogTarget& target);
	//! Disconnects all targets from the log
	void disconnectTargets();
	//! Logs a message
	/*!
	  \param msg Constant reference to the message to log
	*/
	void log(const AbstractLogMessage& msg);
	//! Returns log prefix
	inline const std::string& prefix() const
	{
		return _prefix;
	}
	//! Returns compose source location flag
	inline bool composeSourceLocation() const
	{
		return _copmposeSourceLocation;
	}
private:
	Log(const Log&);							// No copy

	Log& operator=(const Log&);						// No copy
	
	const std::string _prefix;
	const bool _copmposeSourceLocation;

	static LogDispatcher dispatcher;
};

} // namespace isl

#endif


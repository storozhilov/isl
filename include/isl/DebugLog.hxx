#ifndef ISL__DEBUG_LOG__HXX
#define ISL__DEBUG_LOG__HXX

#include <isl/Log.hxx>

namespace isl
{

//! Debug log class which log() method does nothing if ISL_CORE_DEBUGGING macro is undefined
class DebugLog : public Log
{
public:
	//! Default constructor
	DebugLog();
	//! Constructs log
	/*!
	  \param prefix Log prefix wich is to be printed to log target
	  \param composeSourceLocation Print source location to log target if TRUE
	*/
	DebugLog(const std::string& prefix, bool composeSourceLocation = true);
	//! Logs a message
	/*!
	  \param msg Constant reference to the message to log
	*/
	void log(const AbstractLogMessage& msg);
private:
	DebugLog(const Log&);							// No copy

	DebugLog& operator=(const Log&);					// No copy
};

} // namespace isl

#endif

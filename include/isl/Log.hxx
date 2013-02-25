#ifndef ISL__LOG__HXX
#define ISL__LOG__HXX

#include <isl/AbstractLogMessage.hxx>
#include <string>
#include <set>

namespace isl
{

class AbstractLogTarget;

//! Log class
class Log
{
public:
	//! Default constructor
	Log();
	/*!
	  \param prefix Log prefix wich is to be printed to log target
	*/
	Log(const std::string& prefix);
	//! Destructor
	virtual ~Log();

	//! Returns log prefix
	inline const std::string& prefix() const
	{
		return _prefix;
	}

	//! Connects target to log
	/*!
	  \param target Log target to connect to log
	  \note Thread-unsafe
	*/
	void connect(AbstractLogTarget& target);
	//! Disonnects target from log
	/*!
	  \param target Log target to disconnect from log
	  \note Thread-unsafe
	*/
	void disconnect(AbstractLogTarget& target);
	//! Logs a message
	/*!
	  \param msg Constant reference to the message to log
	  \note Thread-safe
	*/
	void log(const AbstractLogMessage& msg);

	//! Returns reference to the ISL error log
	static Log& error();
	//! Returns reference to the ISL warning log
	static Log& warning();
	//! Returns reference to the ISL debug log
	static Log& debug();
private:
	typedef std::set<AbstractLogTarget *> TargetsContainer;

	std::string _prefix;
	TargetsContainer _targets;
};

} // namespace isl

#endif

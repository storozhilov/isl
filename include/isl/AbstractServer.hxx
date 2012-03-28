#ifndef ISL__ABSTRACT_SERVER__HXX
#define ISL__ABSTRACT_SERVER__HXX

#include <isl/WaitCondition.hxx>
#include <isl/AbstractSubsystem.hxx>
#include <vector>
#include <deque>

namespace isl
{

//! Base class for server
/*!
  Starting & stopping server could not be done directly by start() & stop() methods cause it should be done in main thread.
  You should use doStart(), doStop(), doExit() methods which are sends command to the main thread to make an appropriate action.

  TODO Use MessageBus<Command> for commands queue!
*/
class AbstractServer : public AbstractSubsystem
{
public:
	//! Constructor
	/*!
	  \param argc Command-line arguments amount
	  \param argv Command-line arguments array
	*/
	AbstractServer(int argc, char * argv[]);
	//! Runs the server
	/*!
	  Call this method from the main thread only!
	*/
	void run();
	//! Returns command-line arguments amount
	inline unsigned int argc() const
	{
		return _argv.size();
	}
	//! Returns command-line argument value
	inline std::string argv(unsigned int argNo) const
	{
		return _argv.at(argNo);
	}
	//! Sends start command to the server
	void doStart()
	{
		sendCommand(StartCommand);
	}
	//! Sends stop command to the server
	void doStop()
	{
		sendCommand(StopCommand);
	}
	//! Sends exit command to the server
	void doExit()
	{
		sendCommand(ExitCommand);
	}
protected:
	//! Before run event handler
	virtual void beforeRun()
	{}
	//! After run event handler
	virtual void afterRun()
	{}
private:
	enum Command {
		StopCommand,
		StartCommand,
		ExitCommand
	};
	enum PrivateConstants {
		MaxCommandQueueSize = 16
	};

	AbstractServer();
	AbstractServer(const AbstractServer&);						// No copy

	AbstractServer& operator=(const AbstractServer&);				// No copy

	void sendCommand(Command cmd);

	std::vector<std::string> _argv;
	std::deque<Command> _commandsQueue;
	WaitCondition _commandsCond;
};

} // namespace isl

#endif


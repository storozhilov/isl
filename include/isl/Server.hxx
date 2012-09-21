#ifndef ISL__SERVER__HXX
#define ISL__SERVER__HXX

#include <isl/Subsystem.hxx>
#include <isl/SignalSet.hxx>
#include <isl/MessageQueue.hxx>
#include <vector>

namespace isl
{

//! Base class for server
/*!
  Server has a main loop which is to be executed by run() method from the application's main
  thread - this is because UNIX-signals should be blocked in main thread only.
  Main loop is awaits for incoming UNIX-signals or commands and reacts respectively.
*/
class Server : public Subsystem
{
public:
	//! Constructor
	/*!
	  \param argc Command-line arguments amount
	  \param argv Command-line arguments array
	  \param trackSignals UNIX-signals set to track
	  \param clockTimeout Subsystem's clock timeout
	*/
	Server(int argc, char * argv[], const SignalSet& trackSignals = SignalSet(3, SIGHUP, SIGINT, SIGTERM),
			const Timeout& clockTimeout = Timeout::defaultTimeout());
	//! Executes the server
	/*!
	  \note Call this method from the application's main thread only.
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
	//! Returns all command-line arguments
	inline const std::vector<std::string>& argv() const
	{
		return _argv;
	}
	//! Sends restart command to the server
	/*!
	  \return TRUE if the command has been accepted by the server
	*/
	inline bool doRestart()
	{
		return sendCommand(RestartCommand);
	}
	//! Sends terminate command to the server
	/*!
	  \return TRUE if the command has been accepted by the server
	*/
	inline bool doTerminate()
	{
		return sendCommand(TerminateCommand);
	}
protected:
	//! Starting method redefinition to make in protected
	inline void start()
	{
		Subsystem::start();
	}
	//! Stopping method redefinition to make in protected
	inline void stop()
	{
		Subsystem::stop();
	}
	//! Before run event handler
	virtual void beforeRun()
	{}
	//! After run event handler
	virtual void afterRun()
	{}
	//! On signal event handler
	/*!
	  \param signo UNIX-signal number
	  \return TRUE if to continue execution or FALSE otherwise
	*/
	virtual bool onSignal(int signo);
private:
	enum Command {
		RestartCommand,
		TerminateCommand
	};
	Server();
	Server(const Server&);						// No copy

	Server& operator=(const Server&);				// No copy

	inline bool sendCommand(Command cmd)
	{
		return _commandsQueue.push(cmd);
	}

	std::vector<std::string> _argv;
	SignalSet _trackSignals;
	Timeout _clockTimeout;
	MessageQueue<Command> _commandsQueue;
	sigset_t _initialSignalMask;
};

} // namespace isl

#endif

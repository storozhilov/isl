#ifndef ISL__SERVER__HXX
#define ISL__SERVER__HXX

#include <isl/StateSetSubsystem.hxx>
#include <isl/SignalSet.hxx>
#include <vector>

namespace isl
{

//! Base class for server
/*!
  Server has a main loop which is to be executed by run() method from the application's main
  thread - this is because UNIX-signals should be blocked in main thread only.
  Main loop is awaits for incoming UNIX-signals or commands and reacts respectively.
*/
class Server : public StateSetSubsystem
{
public:
	//! Constructor
	/*!
	  \param argc Command-line arguments amount
	  \param argv Command-line arguments array
	  \param trackSignals UNIX-signals set to track (default is to track SIGHUP, SIGINT and SIGTERM)
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
	//! Appoints server restart
	/*!
	  \return TRUE if the command has been accepted by the server
	*/
	void appointRestart();

	//! Daemonizes current process
	static void daemonize();
protected:
	//! Restarts server
	void restart();

	//! Starting method redefinition to make it protected
	virtual void start()
	{
		StateSetSubsystem::start();
	}
	//! Stopping method redefinition to make it protected
	virtual void stop()
	{
		StateSetSubsystem::stop();
	}
	//! On start event handler
	/*!
	  Default implementation does nothing and returns TRUE.
	  \return TRUE if to continue thread execution
	*/
	virtual bool onStart()
	{
		return true;
	}
	//! Doing the work virtual method
	/*!
	  Default implementation does nothing and returns TRUE.
	  \param limit Limit timestamp for doing the work
	  \param stateSet Current state set of the subsystem
	  \return TRUE if to continue thread execution
	*/
	virtual bool doLoad(const Timestamp& limit, const StateSetType::SetType& stateSet)
	{
		return true;
	}
	//! On overload event handler
	/*!
	  Default implementation does nothing and returns TRUE.
	  \param Ticks expired (always > 2)
	  \param stateSet Current state set of the subsystem
	  \return TRUE if to continue server execution
	*/
	virtual bool onOverload(size_t ticksExpired, const StateSetType::SetType& stateSet)
	{
		return true;
	}
	//! On stop event handler
	virtual void onStop()
	{}
	//! On signal event handler
	/*!
	  \param signo UNIX-signal number
	  \return TRUE if to continue execution or FALSE otherwise
	*/
	virtual bool onSignal(int signo);
private:
	Server();
	Server(const Server&);						// No copy

	Server& operator=(const Server&);				// No copy

	bool hasPendingSignals() const;
	int extractPendingSignal() const;

	std::vector<std::string> _argv;
	SignalSet _trackSignals;
	sigset_t _initialSignalMask;
};

} // namespace isl

#endif

#ifndef ISL__SERVER__HXX
#define ISL__SERVER__HXX

#include <isl/Subsystem.hxx>
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
class Server : public Subsystem
{
public:
	//! Restart request inter-thread message
	class RestartRequest : public AbstractThreadMessage
	{
	public:
		RestartRequest() :
			AbstractThreadMessage("Restart request")
		{}

		virtual AbstractThreadMessage * clone() const
		{
			return new RestartRequest(*this);
		}
	};

	//! Constructor
	/*!
	  \note Call this method from the application's main thread only!
	  \param argc Command-line arguments amount
	  \param argv Command-line arguments array
	  \param trackSignals UNIX-signals set to track (default is to track SIGHUP, SIGINT and SIGTERM)
	  \param clockTimeout Subsystem's clock timeout
	*/
	Server(int argc, char * argv[], const SignalSet& trackSignals = SignalSet(3, SIGHUP, SIGINT, SIGTERM),
			const Timeout& clockTimeout = Timeout::defaultTimeout());
	//! Returns a reference to the thread requester
	inline ThreadRequesterType& requester()
	{
		return _requester;
	}
	//! Executes the server
	/*!
	  \note Call this method from the application's main thread only!
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
	 * Could be called from the server's thread or outside.
	 * \note Thread-safe
	 */
	void appointRestart();
	//! Appoints a server termination
	/*!
	 * Could be called from the server's thread or outside.
	 * \note Thread-safe
	 */
	void appointTermination();

	//! Daemonizes current process
	static void daemonize();
protected:
	//! Starting method redefinition to make it protected
	virtual void start()
	{
		Subsystem::start();
	}
	//! Stopping method redefinition to make it protected
	virtual void stop()
	{
		Subsystem::stop();
	}
	//! On start event handler
	virtual void onStart()
	{}
	//! Doing the work virtual method
	/*!
	  \param prevTickTimestamp Previous tick timestamp
	  \param nextTickTimestamp Next tick timestamp
	  \param ticksExpired Amount of expired ticks -> if > 1, then an overload has occured
	*/
	virtual void doLoad(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
	{}
	//! On overload event handler
	/*!
	  Default implementation does nothing and returns TRUE.
	  \param prevTickTimestamp Previous tick timestamp
	  \param nextTickTimestamp Next tick timestamp
	  \param Amount of expired ticks - always > 2
	*/
	virtual void onOverload(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
	{}
	//! On stop event handler
	virtual void onStop()
	{}
	//! On thread request event handler
	/*!
	  \note Default implementation writes an "unrecognized request" entry in the error log
	  \param pendingRequest Constant reference to pending request to process
	  \return Auto-pointer to the response or to 0 if no response has been provided
	*/
	virtual std::auto_ptr<ThreadRequesterType::MessageType> onRequest(const ThreadRequesterType::MessageType& request, bool responseRequired);
	//! On signal event handler
	/*!
	  Default implementation restarts server on SIGHUP signal and terminates it on SIGINT & SIGTERM ones.
	  \param signo UNIX-signal number
	*/
	virtual void onSignal(int signo);
private:
	Server();
	Server(const Server&);							// No copy

	void processSignals();
	//! Awaits for pending thread requests and processes them until limit timestamp has been reached
	/*!
	  \param limit Limit timestamp to await for thread requests
	*/
	void processRequests(const Timestamp& limit);
	//! Processes thread request
	/*!
	  \param pendingRequest Constant reference to pending resuest to process
	  \return Auto-pointer to the response or to 0 if no response has been provided
	*/
	std::auto_ptr<ThreadRequesterType::MessageType> processRequest(const ThreadRequesterType::MessageType& request, bool responseRequired);

	Server& operator=(const Server&);					// No copy

	bool hasPendingSignals() const;
	int extractPendingSignal() const;

	std::vector<std::string> _argv;
	Thread::Handle _threadHandle;
	Subsystem::ThreadRequesterType _requester;
	SignalSet _trackSignals;
	sigset_t _initialSignalMask;
	bool _shouldRestart;
	bool _shouldTerminate;
};

} // namespace isl

#endif

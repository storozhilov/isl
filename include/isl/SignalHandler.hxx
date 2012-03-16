#ifndef ISL__SIGNAL_HANDLER__HXX
#define ISL__SIGNAL_HANDLER__HXX

#include <isl/AbstractSubsystem.hxx>
#include <isl/AbstractServer.hxx>
#include <isl/Thread.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/ReadWriteLock.hxx>
#include <isl/SignalSet.hxx>

namespace isl
{

//! UNIX-signal handler subsystem
class SignalHandler : public AbstractSubsystem
{
public:
	//! Constructor
	/*!
	  \param owner Pointer to owner subsystem
	  \param signalSet Set of UNIX-signals to track
	  \param timeout Sleep timeout
	*/
	SignalHandler(AbstractSubsystem * owner, const SignalSet& signalSet = SignalSet(3, SIGHUP, SIGINT, SIGTERM), const Timeout& timeout = Timeout::defaultTimeout());
	//! Returns
	Timeout timeout() const;
	void setTimeout(const Timeout& newTimeout);
	//! Starts signal handler
	virtual void start();
	//! Stops signal handler
	virtual void stop();
protected:
	//! Returns pointer to AbstractServer's instance among subsystem's owners or 0 if not found
	AbstractServer * findServer();
	//! On signal event handler
	virtual void onSignal(int signo);
private:
	SignalHandler();
	SignalHandler(const SignalHandler&);								// No copy

	SignalHandler& operator=(const SignalHandler&);							// No copy

	class SignalHandlerThread : public Thread
	{
	public:
		SignalHandlerThread(SignalHandler& signalHandler);
	private:
		SignalHandlerThread();
		SignalHandlerThread(const SignalHandlerThread&);					// No copy

		SignalHandlerThread& operator=(const SignalHandlerThread&);				// No copy

		bool hasPendingSignals() const;
		int extractPendingSignal() const;

		virtual void run();

		SignalHandler& _signalHandler;
	};

	Mutex _startStopMutex;
	sigset_t _initialSignalMask;
	SignalSet _blockedSignals;
	Timeout _timeout;
	mutable ReadWriteLock _timeoutRWLock;
	SignalHandlerThread _signalHandlerThread;
};

} // namespace isl

#endif

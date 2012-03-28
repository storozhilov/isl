#ifndef ISL__SIGNAL_HANDLER__HXX
#define ISL__SIGNAL_HANDLER__HXX

#include <isl/AbstractSubsystem.hxx>
#include <isl/AbstractServer.hxx>
#include <isl/Thread.hxx>
#include <isl/SubsystemThread.hxx>
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
	  \param timeout Sleeping timeout
	*/
	SignalHandler(AbstractSubsystem * owner, const SignalSet& signalSet = SignalSet(3, SIGHUP, SIGINT, SIGTERM), const Timeout& timeout = Timeout::defaultTimeout());
	//! Returns sleepeing timeout
	inline Timeout timeout() const
	{
		ReadLocker locker(_timeoutRWLock);
		return _timeout;
	}
	//! Sets sleepeing timeout
	inline void setTimeout(const Timeout& newTimeout)
	{
		WriteLocker locker(_timeoutRWLock);
		_timeout = newTimeout;
	}
protected:
	//! Returns pointer to AbstractServer's instance among subsystem's owners or 0 if not found
	AbstractServer * findServer();
	//! On signal event handler
	virtual void onSignal(int signo);
private:
	SignalHandler();
	SignalHandler(const SignalHandler&);								// No copy

	SignalHandler& operator=(const SignalHandler&);							// No copy

	class SignalHandlerThread : public SubsystemThread
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

	virtual void beforeStart();
	virtual void afterStart();
	virtual void beforeStop();
	virtual void afterStop();

	sigset_t _initialSignalMask;
	SignalSet _blockedSignals;
	Timeout _timeout;
	mutable ReadWriteLock _timeoutRWLock;
	SignalHandlerThread _signalHandlerThread;
};

} // namespace isl

#endif

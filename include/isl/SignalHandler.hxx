#ifndef ISL__SIGNAL_HANDLER__HXX
#define ISL__SIGNAL_HANDLER__HXX

#include <isl/Subsystem.hxx>
#include <isl/Server.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/ReadWriteLock.hxx>
#include <isl/SignalSet.hxx>

namespace isl
{

//! UNIX signal handler subsystem
/*!
  TODO Migration from sigpending(2)/sigwait(P) to sigtimedwait(2)
*/
class SignalHandler : public Subsystem
{
public:
	//! Constructor
	/*!
	  \param owner Pointer to owner subsystem
	  \param signalSet Set of UNIX signals to track
	  \param timeout Awaiting for signal timeout
	*/
	SignalHandler(Subsystem * owner, const SignalSet& signalSet = SignalSet(3, SIGHUP, SIGINT, SIGTERM), const Timeout& timeout = Timeout::defaultTimeout());
	//! Returns awaiting for signal timeout
	inline Timeout timeout() const
	{
		ReadLocker locker(runtimeParamsRWLock);
		return _timeout;
	}
	//! Sets awaiting for signal timeout
	/*!
	  Subsystem's restart needed to completely apply the new value;

	  \param newTimeout New awaiting for signal timeout
	*/
	inline void setTimeout(const Timeout& newTimeout)
	{
		ReadLocker locker(runtimeParamsRWLock);
		_timeout = newTimeout;
	}
protected:
	//! Returns pointer to Server's instance among subsystem's owners or 0 if not found
	Server * findServer();
	//! On signal event handler
	/*!
	  \param signo UNIX signal to process
	*/
	virtual void onSignal(int signo);
private:
	SignalHandler();
	SignalHandler(const SignalHandler&);								// No copy

	SignalHandler& operator=(const SignalHandler&);							// No copy

	class SignalHandlerThread : public AbstractThread
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
	SignalHandlerThread _signalHandlerThread;
};

} // namespace isl

#endif

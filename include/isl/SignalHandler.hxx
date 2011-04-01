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

/*------------------------------------------------------------------------------
   NOTE! SignalHandler subsystem must be started BEFORE any other
   threads/subsystems are started to be able to clear signal mask of the main
   thread of the server!
------------------------------------------------------------------------------*/

class SignalHandler : public AbstractSubsystem
{
public:
	SignalHandler(AbstractServer& server, const SignalSet& signalSet = SignalSet(3, SIGHUP, SIGINT, SIGTERM));

	Timeout timeout() const;
	void setTimeout(const Timeout& newTimeout);
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
		WaitCondition _timeoutCond;
	};

	virtual void onStartCommand();
	virtual void onStopCommand();
	virtual bool onSignal(int signo);								// Returns true if to continue subsystem's execution

	AbstractServer& _server;
	sigset_t _initialSignalMask;
	SignalSet _blockedSignals;
	Timeout _timeout;
	mutable ReadWriteLock _timeoutRWLock;
	SignalHandlerThread _signalHandlerThread;
};

} // namespace isl

#endif

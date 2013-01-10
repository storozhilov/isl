#ifndef ISL__STATE_SET_SUBSYSTEM__HXX
#define ISL__STATE_SET_SUBSYSTEM__HXX

#include <isl/Subsystem.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/AbstractThread.hxx>
#include <isl/StateSet.hxx>

namespace isl
{

//! Subsystem, which is controlled by StateSet object and is starting/stopping it's threads
class StateSetSubsystem : public Subsystem
{
public:
	StateSetSubsystem(Subsystem * owner, const Timeout& clockTimeout = Timeout::defaultTimeout());
	//! Starting subsystem virtual method
	/*!
	  Default implementation starts all children subsystems and subsystem's threads
	  \note Thread-unsafe
	*/
	virtual void start();
	//! Stopping subsystem and awaiting for it's termination virtual method
	/*!
	  Default implementation stops all subsystem's threads and children subsystems
	  \note Thread-unsafe
	*/
	virtual void stop();
protected:
	//! State set subsystem state
	enum State {
		TerminationState,
		RestartState
	};
	//! State set subsystem state set type
	typedef StateSet<State> StateSetType;
	//! State set subsystem controllable abstract thread
	class AbstractThread : public ::isl::AbstractThread
	{
	public:
		//! Constructs subsystem-controllable thread, which is controlled by the inter-thread 
		/*!
		  \param subsystem Reference to subsystem object new thread is controlled by
		  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
		  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
		*/
		AbstractThread(StateSetSubsystem& subsystem, bool isTrackable = false, bool awaitStartup = false);
		//! Destructor
		virtual ~AbstractThread();
		//! Returns a reference to the state set subsystem
		StateSetSubsystem& subsystem()
		{
			return _subsystem;
		}
	protected:
		//! Returns TRUE if the thread should be terminated
		bool shouldTerminate();
		//! Awaits for termination
		/*!
		  \param limit Limit timestamp to wait until the termination
		  \returns TRUE if the thread should be terminated
		*/
		bool awaitTermination(const Timestamp& limit);
		//! Awaits for termination
		/*!
		  \param timeout Timeout to wait for the termination
		  \param timeoutLeft Memory location where time interval which is remains after awaiting for termination or 0 otherwise
		  \returns TRUE if the thread should be terminated
		*/
		inline bool awaitTermination(const Timeout& timeout, Timeout * timeoutLeft = 0);
	private:
		StateSetSubsystem& _subsystem;
	};
	//! Returns reference to the state set subsystem's state set
	StateSetType& stateSet()
	{
		return _stateSet;
	}
	//! Appoints a state set subsystem's termination by setting appropriate item in state set
	void appointTermination();
	//! Starts state set subsystem's threads
	void startThreads();
	//! Stops state set subsystem's threads
	void stopThreads();
private:
	StateSetSubsystem();
	StateSetSubsystem(const StateSetSubsystem&);							// No copy

	StateSetSubsystem& operator=(const StateSetSubsystem&);						// No copy

	typedef std::list<AbstractThread *> Threads;

	void registerThread(AbstractThread * thread);
	void unregisterThread(AbstractThread * thread);

	StateSetType _stateSet;
	Threads _threads;
};

} // namespace isl

#endif

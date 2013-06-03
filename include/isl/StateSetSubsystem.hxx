#ifndef ISL__STATE_SET_SUBSYSTEM__HXX
#define ISL__STATE_SET_SUBSYSTEM__HXX

#include <isl/Subsystem.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/AbstractThread.hxx>
#include <isl/StateSet.hxx>

namespace isl
{

//! Subsystem, which is controlling it's threads using internal StateSet object
/*!
  TODO: To be removed (\sa Subsystem)
 */ 
class StateSetSubsystem : public Subsystem
{
public:
	//! Constructs a new state set subsystem
	/*!
	  \param owner The owner subsystem of the new subsystem
	  \param clockTimeout Subsystem's clock timeout
	 */
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
		//! Constructs state set subsystem controllable thread
		/*!
		  \param subsystem Reference to state set subsystem object new thread is controlled by
		  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
		  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
		*/
		AbstractThread(StateSetSubsystem& subsystem, bool isTrackable = false, bool awaitStartup = false);
		//! Destructor
		virtual ~AbstractThread();
		//! Returns a reference to the state set subsystem
		inline StateSetSubsystem& subsystem()
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
	//! State set subsystem controllable thread with main loop
	class Thread : public AbstractThread
	{
	public:
		//! Constructs state set subsystem controllable thread
		/*!
		  \param subsystem Reference to state set subsystem object new thread is controlled by
		  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
		  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
		*/
		Thread(StateSetSubsystem& subsystem, bool isTrackable = false, bool awaitStartup = false);
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
		  \return TRUE if to continue thread execution
		*/
		virtual bool onOverload(size_t ticksExpired, const StateSetType::SetType& stateSet)
		{
			return true;
		}
		//! On stop event handler
		virtual void onStop()
		{}
	private:
		//! Thread execution virtual method redefinition
		virtual void run();
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

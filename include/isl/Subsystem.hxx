#ifndef ISL__SUBSYSTEM__HXX
#define ISL__SUBSYSTEM__HXX

#include <isl/Timeout.hxx>
#include <isl/AbstractThread.hxx>
#include <isl/StateSet.hxx>
#include <list>

namespace isl
{

/*!
  Basic component of any server according to <a href="http://en.wikipedia.org/wiki/Composite_pattern">Composite Design Pattern</a>.
  TODO: To implement the following design:
  - Subsystem::AbstractThread
    - appointTermination()
  - Subsystem::AbstractStateSetThread;
    - onStateSetChanged()
  - Subsystem::AbstractRequesterThread;
    - onRequestReceived()
 */ 
class Subsystem
{
public:
	//! Constructs a new subsystem
	/*!
	  \param owner The owner subsystem of the new subsystem
	  \param clockTimeout Subsystem's clock timeout
	 */
	Subsystem(Subsystem * owner, const Timeout& clockTimeout = Timeout::defaultTimeout());
	//! Destructor
	virtual ~Subsystem();
	//! Returns an owner of the subsystem
	/*!
	  \note Thread-safe
	*/
	inline Subsystem * owner() const
	{
		return _owner;
	}
	//! Returns clock timeout
	inline const Timeout& clockTimeout() const
	{
		return _clockTimeout;
	}
	//! Sets new clock timeout
	/*!
	  \param newValue New clock timeout value

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	inline void setClockTimeout(const Timeout& newValue)
	{
		_clockTimeout = newValue;
	}
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
	//! Starts children subsystems
	void startChildren();
	//! Stops children subsystems
	void stopChildren();
private:
	Subsystem();
	Subsystem(const Subsystem&);							// No copy

	Subsystem& operator=(const Subsystem&);						// No copy

	typedef std::list<Subsystem *> Children;

	void registerChild(Subsystem * child);
	void unregisterChild(Subsystem * child);

	Subsystem * _owner;
	Timeout _clockTimeout;
	Children _children;
};

//! Server subsystem base class
/*!
  Basic component of any server according to <a href="http://en.wikipedia.org/wiki/Composite_pattern">Composite Design Pattern</a>.
  TODO: To implement the following design:
  - Subsystem::AbstractThread
    - appointTermination()
  - Subsystem::AbstractStateSetThread;
    - onStateSetChanged()
  - Subsystem::AbstractRequesterThread;
    - onRequestReceived()
 */ 
class Subsystem_NEW
{
public:
	//! Abstract thread, which is controllable by a subsystem
	class AbstractThread : public ::isl::AbstractThread
	{
	public:
		//! Constructs a thread
		/*!
		  \param subsystem Reference to the subsystem object a thread is to be controlled by
		  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
		  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
		*/
		AbstractThread(Subsystem_NEW& subsystem, bool isTrackable = false, bool awaitStartup = false) :
			::isl::AbstractThread(isTrackable, awaitStartup),
			_subsystem(subsystem)
		{
			_subsystem.registerThread(this);
		}
		virtual ~AbstractThread()
		{
			_subsystem.unregisterThread(this);
		}
		//! Returns a reference to the subsystem
		inline Subsystem_NEW& subsystem()
		{
			return _subsystem;
		}
	protected:
		//! Awaits for termination
		/*!
		  \param timeout Timeout to wait for the termination
		  \param timeoutLeft Memory location where time interval which is remains after awaiting for termination or 0 otherwise
		  \returns TRUE if the thread should be terminated
		*/
		bool awaitTermination(const Timeout& timeout, Timeout * timeoutLeft = 0)
		{
			Timestamp limit = Timestamp::limit(timeout);
			bool result = awaitTermination(limit);
			if (timeoutLeft) {
				*timeoutLeft = result ? limit.leftTo() : Timeout();
			}
			return result;
		}

		//! Resets thread (called before start)
		virtual void reset() = 0;
		//! Appoints a subsystem's thread termination by setting appropriate item in state set
		virtual void appointTermination() = 0;
		//! Returns TRUE if the thread should be terminated
		virtual bool shouldTerminate() = 0;
		//! Awaits for termination
		/*!
		  \param limit Limit timestamp to wait until the termination
		  \returns TRUE if the thread should be terminated
		*/
		virtual bool awaitTermination(const Timestamp& limit) = 0;
	private:
		Subsystem_NEW& _subsystem;

		friend class Subsystem_NEW;
	};
	//! State-set controllable abstract thread
	class AbstractStateSetThread : public AbstractThread
	{
	public:
		//! State-set controllable thread
		enum State {
			TerminationState,		//!< Termination state
			//RestartState,			//!< Restart state (TODO: ???)
			UserState			//!< Start of user states space
		};
		//! State set subsystem state set type
		typedef StateSet<State> StateSetType;
		//! Constructs a thread
		/*!
		  \param subsystem Reference to the subsystem object a thread is to be controlled by
		  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
		  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
		*/
		AbstractStateSetThread(Subsystem_NEW& subsystem, bool isTrackable = false, bool awaitStartup = false) :
			AbstractThread(subsystem, isTrackable, awaitStartup),
			_stateSet()
		{}
		//! Returns reference to the state set subsystem's state set
		StateSetType& stateSet()
		{
			return _stateSet;
		}
	protected:
		//! Resets thread (called before start)
		virtual void reset()
		{
			_stateSet.reset();
		}
		//! Appoints a subsystem's thread termination by setting appropriate item in state set
		virtual void appointTermination()
		{
			_stateSet.insert(TerminationState);
		}
		//! Returns TRUE if the thread should be terminated
		virtual bool shouldTerminate()
		{
			StateSetType::SetType set = _stateSet.fetch();
			return set.find(TerminationState) != set.end();
		}
		//! Awaits for termination
		/*!
		  \param limit Limit timestamp to wait until the termination
		  \returns TRUE if the thread should be terminated
		*/
		virtual bool awaitTermination(const Timestamp& limit)
		{
			bool shouldTerminate;
			_stateSet.await(TerminationState, limit, &shouldTerminate);
			return shouldTerminate;
		}
	private:
		StateSetType _stateSet;
	};
	//! Constructs a new subsystem
	/*!
	  \param owner The owner subsystem of the new subsystem
	  \param clockTimeout Subsystem_NEW's clock timeout
	 */
	Subsystem_NEW(Subsystem_NEW * owner, const Timeout& clockTimeout = Timeout::defaultTimeout());
	//! Destructor
	virtual ~Subsystem_NEW();
	//! Returns an owner of the subsystem
	/*!
	  \note Thread-safe
	*/
	inline Subsystem_NEW * owner() const
	{
		return _owner;
	}
	//! Returns clock timeout
	inline const Timeout& clockTimeout() const
	{
		return _clockTimeout;
	}
	//! Sets new clock timeout
	/*!
	  \param newValue New clock timeout value

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	inline void setClockTimeout(const Timeout& newValue)
	{
		_clockTimeout = newValue;
	}
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
	//! Starts children subsystems
	void startChildren();
	//! Stops children subsystems
	void stopChildren();
	//! Starts state set subsystem's threads
	void startThreads();
	//! Stops state set subsystem's threads
	void stopThreads();
private:
	Subsystem_NEW();
	Subsystem_NEW(const Subsystem_NEW&);							// No copy

	Subsystem_NEW& operator=(const Subsystem_NEW&);						// No copy

	typedef std::list<Subsystem_NEW *> Children;

	void registerChild(Subsystem_NEW * child);
	void unregisterChild(Subsystem_NEW * child);

	typedef std::list<AbstractThread *> Threads;

	void registerThread(AbstractThread * thread);
	void unregisterThread(AbstractThread * thread);

	Subsystem_NEW * _owner;
	Timeout _clockTimeout;
	Children _children;
	Threads _threads;
};

} // namespace isl

#endif

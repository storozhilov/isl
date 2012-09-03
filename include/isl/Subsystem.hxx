#ifndef ISL__SUBSYSTEM__HXX
#define ISL__SUBSYSTEM__HXX

#include <isl/ReadWriteLock.hxx>
#include <isl/Mutex.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Debug.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/AbstractThread.hxx>
#include <string>
#include <algorithm>

namespace isl
{

//! Server subsystem base class
/*!
  Basic component of any server acconrding to <a href="http://en.wikipedia.org/wiki/Composite_pattern">Composite Design Pattern</a>.
*/
class Subsystem
{
public:
	//! Subsystem state constants
	enum State {
		IdlingState,		//!< Subsystem is idling
		StartingState,		//!< Subsystem is starting up
		RunningState,		//!< Subsystem is running
		StoppingState		//!< Subsystem is shutting down
	};
protected:
	//! Subsystem-aware abstract thread with is controlled by it's subsystem
	class AbstractThread : public ::isl::AbstractThread
	{
	public:
		//! Constructs subsystem-aware thread
		/*!
		  \param subsystem Reference to subsystem object new thread is controlled by
		  \param autoStop Thread will be terminated on subsytem's stop operation if TRUE
		  \param awaitStartup Await for thread to be started
		*/
		AbstractThread(Subsystem& subsystem, bool autoStop = true, bool awaitStartup = false);
		virtual ~AbstractThread();
		//! Returns reference to subsystem object
		inline Subsystem& subsystem() const
		{
			return _subsystem;
		}
		inline bool autoStop() const
		{
			return _autoStop;
		}
		//! Returns TRUE if the thread should be terminated due to it's subsystem state
		bool shouldTerminate();
		//! Awaiting for thread termination method
		/*!
		  \param timeout Timeout to wait
		  \return TRUE if the thread has been terminated
		*/
		bool awaitShouldTerminate(Timeout timeout = Timeout::defaultTimeout());
		//! Sets should terminate flag to the new value
		/*!
		  \param newValue New value for the should terminate flag
		*/
		void setShouldTerminate(bool newValue);
	private:
		Subsystem& _subsystem;
		bool _autoStop;
		bool _shouldTerminate;
		WaitCondition _shouldTerminateCond;
	};
	//! State operations utility class
	class StateLocker
	{
	public:
		StateLocker(Subsystem& subsystem) :
			_subsystem(subsystem),
			_locker(subsystem._stateCond.mutex())
		{}

		inline State state() const
		{
			return _subsystem._state;
		}
		inline State setState(State newState)
		{
			State oldState = _subsystem._state;
			_subsystem.setStateUnsafe(newState);
			return oldState;
		}
		inline bool awaitStateChange(const Timeout& timeout)
		{
			return _subsystem._stateCond.wait(timeout);
		}
	private:
		StateLocker();
		StateLocker(const StateLocker&);

		StateLocker& operator=(const StateLocker&);

		Subsystem& _subsystem;
		MutexLocker _locker;
	};

public:
	//! Constructs a new subsystem
	/*!
	  \param owner The owner subsystem of the new subsystem
	*/
	Subsystem(Subsystem * owner);
	//! Destructor
	virtual ~Subsystem();
	//! Returns an owner of the subsystem
	inline Subsystem * owner() const
	{
		return _owner;
	}
	//! Thread-safely returns subsystem's state
	inline State state() const
	{
		MutexLocker locker(_stateCond.mutex());
		return _state;
	}
	//! Asynchronously starts subsystem
	void start();
	//! Synchronously stops subsystem
	void stop();
protected:
	//! Before start event handler
	virtual void beforeStart()
	{}
	//! Starts children and threads
	virtual void startImpl();
	//! After start event handler
	virtual void afterStart()
	{}
	//! Before stop event handler
	virtual void beforeStop()
	{}
	//! Stops children and threads
	virtual void stopImpl();
	//! After stop event handler
	virtual void afterStop()
	{}
private:
	Subsystem();
	Subsystem(const Subsystem&);							// No copy

	Subsystem& operator=(const Subsystem&);						// No copy

	typedef std::list<Subsystem *> Children;
	typedef std::list<AbstractThread *> Threads;

	inline void setStateUnsafe(State newState)
	{
		if (_state != newState) {
			_state = newState;
			_stateCond.wakeAll();
		}
	}
	void registerChild(Subsystem * child);
	void unregisterChild(Subsystem * child);
	void registerThread(AbstractThread * thread);
	void unregisterThread(AbstractThread * thread);

	Subsystem * _owner;
	Children _children;
	Threads _threads;
	State _state;
	mutable WaitCondition _stateCond;
};

} // namespace isl

#endif

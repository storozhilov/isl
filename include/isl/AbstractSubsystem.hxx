#ifndef ISL__ABSTRACT_SUBSYSTEM__HXX
#define ISL__ABSTRACT_SUBSYSTEM__HXX

#include <isl/ReadWriteLock.hxx>
#include <isl/Mutex.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Debug.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/SubsystemThread.hxx>
#include <string>
#include <algorithm>

namespace isl
{

//! Subsystem base class
class AbstractSubsystem
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
	//! State operations utility class
	class StateLocker
	{
	public:
		StateLocker(AbstractSubsystem& subsystem) :
			_subsystem(subsystem),
			_locker(subsystem._stateCond.mutex())
		{}

		State state() const
		{
			return _subsystem._state;
		}
		State setState(State newState)
		{
			State oldState = _subsystem._state;
			_subsystem.setStateUnsafe(newState);
			return oldState;
		}
		bool awaitStateChange(const Timeout& timeout)
		{
			return _subsystem._stateCond.wait(timeout);
		}
	private:
		StateLocker();
		StateLocker(const StateLocker&);

		StateLocker& operator=(const StateLocker&);

		AbstractSubsystem& _subsystem;
		MutexLocker _locker;
	};

public:
	//! Constructs a new subsystem
	/*!
	  \param owner The owner subsystem of the new subsystem
	*/
	AbstractSubsystem(AbstractSubsystem * owner) :
		_owner(owner),
		_children(),
		_threads(),
		_state(IdlingState),
		_stateCond()
	{
		if (_owner) {
			_owner->registerChild(this);
		}
	}
	//! Virtual destructor cause class is virtual
	virtual ~AbstractSubsystem()
	{
		if (_owner) {
			_owner->unregisterChild(this);
		}
		for (Children::iterator i = _children.begin(); i != _children.end(); ++i) {
			// TODO Warning log message?
			(*i)->_owner = 0;
		}
	}
	//! Returns an owner of the subsystem
	inline AbstractSubsystem * owner() const
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
	void start()
	{
		MutexLocker locker(_stateCond.mutex());
		if (_state != IdlingState) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Subsystem is not idling before start"));
		}
		setStateUnsafe(StartingState);
		beforeStart();
		startImpl();
		afterStart();
		setStateUnsafe(RunningState);
	}
	//! Synchronously stops subsystem
	void stop()
	{
		MutexLocker locker(_stateCond.mutex());
		setStateUnsafe(StoppingState);
		beforeStop();
		stopImpl();
		afterStop();
		setStateUnsafe(IdlingState);
	}
	//! Returns state name by state value
	static const char * stateName(State state)
	{
		switch (state) {
			case IdlingState:
				return IdlingStateName;
				break;
			case StartingState:
				return StartingStateName;
				break;
			case RunningState:
				return RunningStateName;
				break;
			case StoppingState:
				return StoppingStateName;
				break;
			default:
				return NotDefinedStateName;
		}
	}
protected:
	//! Before start event handler
	virtual void beforeStart()
	{}
	//! Starts children and threads
	virtual void startImpl()
	{
		// Starting children subsystems
		for (Children::iterator i = _children.begin(); i != _children.end(); ++i) {
			(*i)->start();
		}
		// Starting threads
		for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
			(*i)->setShouldTerminate(false);
			(*i)->start();
		}
	}
	//! After start event handler
	virtual void afterStart()
	{}
	//! Before stop event handler
	virtual void beforeStop()
	{}
	//! Stops children and threads
	virtual void stopImpl()
	{
		// Stopping threads (TODO Timed join & killing thread if it has not been terminated)
		for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
			if ((*i)->_autoStop) {
				(*i)->setShouldTerminate(true);
			}
			(*i)->join();
		}
		// Stopping chldren subsystems
		for (Children::iterator i = _children.begin(); i != _children.end(); ++i) {
			(*i)->stop();
		}
	}
	//! After stop event handler
	virtual void afterStop()
	{}
private:
	AbstractSubsystem();
	AbstractSubsystem(const AbstractSubsystem&);							// No copy

	AbstractSubsystem& operator=(const AbstractSubsystem&);						// No copy

	typedef std::list<AbstractSubsystem *> Children;
	typedef std::list<SubsystemThread *> Threads;

	void setStateUnsafe(State newState)
	{
		if (_state != newState) {
			_state = newState;
			_stateCond.wakeAll();
		}
	}

	void registerChild(AbstractSubsystem * child)
	{
		if (std::find(_children.begin(), _children.end(), child) != _children.end()) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Child subsystem has been already registered in subsystem"));
		}
		_children.push_back(child);
	}
	void unregisterChild(AbstractSubsystem * child)
	{
		Children::iterator childPos = std::find(_children.begin(), _children.end(), child);
		if (childPos == _children.end()) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Child subsystem have not been registered in subsystem"));
		}
		_children.erase(childPos);
	}
	void registerThread(SubsystemThread * thread)
	{
		if (std::find(_threads.begin(), _threads.end(), thread) != _threads.end()) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread has been already registered in subsystem"));
		}
		_threads.push_back(thread);
	}
	void unregisterThread(SubsystemThread * thread)
	{
		Threads::iterator threadPos = std::find(_threads.begin(), _threads.end(), thread);
		if (threadPos == _threads.end()) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Child subsystem have not been registered in subsystem"));
		}
		_threads.erase(threadPos);
	}

	AbstractSubsystem * _owner;
	Children _children;
	Threads _threads;
	State _state;
	mutable WaitCondition _stateCond;

	static const char NotDefinedStateName[];
	static const char IdlingStateName[];
	static const char StartingStateName[];
	static const char RunningStateName[];
	static const char RestartingStateName[];
	static const char StoppingStateName[];

	friend class SubsystemThread;
};

} // namespace isl

#endif


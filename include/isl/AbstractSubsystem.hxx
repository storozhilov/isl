#ifndef ISL__ABSTRACT_SUBSYSTEM__HXX
#define ISL__ABSTRACT_SUBSYSTEM__HXX

#include <isl/ReadWriteLock.hxx>
#include <isl/Mutex.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Core.hxx>
#include <isl/Debug.hxx>
#include <isl/Error.hxx>
#include <isl/SubsystemThread.hxx>
#include <string>
#include <set>

#include <algorithm>

#include <stdexcept>

namespace isl
{

//! Subsystem base class
/*!
  TODO Maybe bool shouldTerminate() method?
*/
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
	typedef std::set<State> StateSet;
	//! Constructs a new subsystem
	/*!
	  \param owner The owner subsystem of the new subsystem
	*/
	AbstractSubsystem(AbstractSubsystem * owner) :
		_owner(owner),
		_children(),
		_threads(),
		_state(IdlingState),
		_stateCond(),
		_startStopMutex()
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
	//! Returns TRUE if subsystem is idling
	inline bool isIdling() const
	{
		MutexLocker locker(_stateCond.mutex());
		return _state == IdlingState;
	}
	//! Returns TRUE if subsystem is starting
	inline bool isStarting() const
	{
		MutexLocker locker(_stateCond.mutex());
		return _state == StartingState;
	}
	//! Returns TRUE if subsystem is running
	inline bool isRunning() const
	{
		MutexLocker locker(_stateCond.mutex());
		return _state == RunningState;
	}
	//! Returns TRUE if subsystem is stopping
	inline bool isStopping() const
	{
		MutexLocker locker(_stateCond.mutex());
		return _state == StoppingState;
	}
	//! Returns TRUE if subsystem is stopping or stopped
	inline bool shouldTerminate() const
	{
		MutexLocker locker(_stateCond.mutex());
		return _state == StoppingState || _state == IdlingState;
	}
	//! Inspects subsystem's state to be equal to passed one and returns immediately or waits for state to be changed during timeout otherwise
	/*!
	  \param state State to inspect for
	  \param timeout Timeout to wait for new state
	  \return Subsystems's state
	*/
	inline State awaitState(State state, Timeout timeout)
	{
		MutexLocker locker(_stateCond.mutex());
		if (_state == state) {
			return _state;
		}
		_stateCond.wait(timeout);
		return _state;
	}
	//! Inspects subsystem's state to be one of the passed ones and returns immediately or waits for state to be changed during timeout otherwise
	/*!
	  \param stateSet States to inspect for
	  \param timeout Timeout to wait for new state
	  \return Subsystems's state
	*/
	inline State awaitState(const StateSet& stateSet, Timeout timeout)
	{
		MutexLocker locker(_stateCond.mutex());
		if (stateSet.find(_state) != stateSet.end()) {
			return _state;
		}
		_stateCond.wait(timeout);
		return _state;
	}
	//! Inspects subsystem's state to be not equal to passed one and returns immediately or waits for state to be changed during timeout otherwise
	/*!
	  \param state State to inspect for
	  \param timeout Timeout to wait for new state
	  \return Subsystems's state
	*/
	inline State awaitNotState(State state, Timeout timeout)
	{
		MutexLocker locker(_stateCond.mutex());
		if (_state != state) {
			return _state;
		}
		_stateCond.wait(timeout);
		return _state;
	}
	//! Inspects subsystem's state to not to be one of the passed ones and returns immediately or waits for state to be changed during timeout otherwise
	/*!
	  \param stateSet States to inspect for
	  \param timeout Timeout to wait for new state
	  \return Subsystems's state
	*/
	inline State awaitNotState(const StateSet& stateSet, Timeout timeout)
	{
		MutexLocker locker(_stateCond.mutex());
		if (stateSet.find(_state) == stateSet.end()) {
			return _state;
		}
		_stateCond.wait(timeout);
		return _state;
	}
	//! Asynchronously starts subsystem
	void start()
	{
		MutexLocker locker(_startStopMutex);
		beforeStart();
		startImpl();
		afterStart();
	}
	//! Synchronously stops subsystem
	void stop()
	{
		MutexLocker locker(_startStopMutex);
		beforeStop();
		stopImpl();
		afterStop();
	}
	//! Returns state name by state value
	static const wchar_t * stateName(State state)
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
	inline Mutex& startStopMutex()
	{
		return _startStopMutex;
	}
	//! Thread-safely sets subsystem's state
	inline State setState(State newState)
	{
		MutexLocker locker(_stateCond.mutex());
		State oldState = _state;
		if (newState != oldState) {
			_state = newState;
			_stateCond.wakeAll();
		}
		return oldState;
	}
	//! Thread-safely changes subsystem's state from one to another
	/*!
	  \param oldState Subsystem's state to switch from
	  \param newState Subsystem's state to switch to
	*/
	inline void setState(State oldState, State newState)
	{
		MutexLocker locker(_stateCond.mutex());
		if (_state != oldState) {
			// TODO Use isl::Exception class
			throw new std::runtime_error("Invalid subsystem state to switch from");
		}
		_state = newState;
		_stateCond.wakeAll();
	}

	virtual void beforeStart()
	{}

	virtual void startImpl()
	{
		setState(IdlingState, StartingState);
		// Starting children subsystems
		for (Children::iterator i = _children.begin(); i != _children.end(); ++i) {
			(*i)->start();
		}
		// Starting threads
		for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
			(*i)->start();
		}
		setState(StartingState, RunningState);
	}

	virtual void afterStart()
	{}

	virtual void beforeStop()
	{}

	virtual void stopImpl()
	{
		setState(StoppingState);
		// Stopping threads (TODO Timed join & killing thread if it has not been terminated)
		for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
			(*i)->join();
		}
		// Stopping chldren subsystems
		for (Children::iterator i = _children.begin(); i != _children.end(); ++i) {
			(*i)->stop();
		}
		setState(IdlingState);
	}

	virtual void afterStop()
	{}
private:
	AbstractSubsystem();
	AbstractSubsystem(const AbstractSubsystem&);							// No copy

	AbstractSubsystem& operator=(const AbstractSubsystem&);						// No copy

	typedef std::list<AbstractSubsystem *> Children;
	typedef std::list<SubsystemThread *> Threads;

	void registerChild(AbstractSubsystem * child)
	{
		if (std::find(_children.begin(), _children.end(), child) != _children.end()) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Child subsystem has been already registered in subsystem"));
		}
		_children.push_back(child);
	}
	void unregisterChild(AbstractSubsystem * child)
	{
		Children::iterator childPos = std::find(_children.begin(), _children.end(), child);
		if (childPos == _children.end()) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Child subsystem have not been registered in subsystem"));
		}
		_children.erase(childPos);
	}
	void registerThread(SubsystemThread * thread)
	{
		if (std::find(_threads.begin(), _threads.end(), thread) != _threads.end()) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Thread has been already registered in subsystem"));
		}
		_threads.push_back(thread);
	}
	void unregisterThread(SubsystemThread * thread)
	{
		Threads::iterator threadPos = std::find(_threads.begin(), _threads.end(), thread);
		if (threadPos == _threads.end()) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Child subsystem have not been registered in subsystem"));
		}
		_threads.erase(threadPos);
	}

	AbstractSubsystem * _owner;
	Children _children;
	Threads _threads;
	State _state;
	mutable WaitCondition _stateCond;
	Mutex _startStopMutex;

	static const wchar_t NotDefinedStateName[];
	static const wchar_t IdlingStateName[];
	static const wchar_t StartingStateName[];
	static const wchar_t RunningStateName[];
	static const wchar_t RestartingStateName[];
	static const wchar_t StoppingStateName[];

	friend class SubsystemThread;
};

} // namespace isl

#endif


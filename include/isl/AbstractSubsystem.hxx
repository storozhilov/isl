#ifndef ISL__ABSTRACT_SUBSYSTEM__HXX
#define ISL__ABSTRACT_SUBSYSTEM__HXX

#include <isl/ReadWriteLock.hxx>
#include <isl/Mutex.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Core.hxx>
#include <isl/Debug.hxx>
#include <string>
#include <set>

#include <stdexcept>

namespace isl
{

//! Subsystem generalization class
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
	  TODO Registration in the owner subsystem
	*/
	AbstractSubsystem(AbstractSubsystem * owner) :
		_owner(owner),
		_state(IdlingState),
		_stateCond()
	{}
	//! Virtual destructor cause class is virtual
	/*!
	  TODO Unregistration in the owner subsystem
	*/
	virtual ~AbstractSubsystem()
	{}
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
	//! Awaits once for subsystem's state to be set
	/*!
	  \param timeout Timeout of waiting for state
	  \return Finally inspected state
	*/
	/*inline State awaitStateChange(Timeout timeout)
	{

		MutexLocker locker(_stateCond.mutex());
		_stateCond.wait(timeout);
		return _state;
	}*/
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
	//! Asynchronously starting subsystem abstract virtual method to override in descendants
	/*!
	  All state manipulations are under this method's control
	  \return True if the starting process has been successfully launched
	  TODO Add default implementation, which starts all children subsystems?
	*/
	virtual void start() = 0;
	//! Synchronously stopping subsystem abstract virtual method to override in descendants
	/*!
	  All state manipulations are under this method's control
	  TODO Add default implementation, which stops all children subsystems?
	*/
	virtual void stop() = 0;
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
	//! Thread-safely sets subsystem's state from one to another
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
private:
	AbstractSubsystem();
	AbstractSubsystem(const AbstractSubsystem&);							// No copy

	AbstractSubsystem& operator=(const AbstractSubsystem&);						// No copy

	AbstractSubsystem * _owner;
	State _state;
	mutable WaitCondition _stateCond;

	static const wchar_t NotDefinedStateName[];
	static const wchar_t IdlingStateName[];
	static const wchar_t StartingStateName[];
	static const wchar_t RunningStateName[];
	static const wchar_t RestartingStateName[];
	static const wchar_t StoppingStateName[];
};

////! Subsystem generalization class
//class AbstractSubsystem
//{
//public:
//	//! Subsystem state constants
//	enum State {
//		IdlingState,		//!< Subsystem is idling
//		StartingState,		//!< Subsystem is starting up
//		RunningState,		//!< Subsystem is running
//		StoppingState		//!< Subsystem is shutting down
//	};
//	//! Constructs a new subsystem
//	/*!
//	  \param owner The owner subsystem of the new subsystem
//	  TODO Registration in the owner subsystem
//	*/
//	AbstractSubsystem(AbstractSubsystem * owner) :
//		_owner(owner),
//		_state(IdlingState),
//		_stateCond()
//	{}
//	//! Virtual destructor cause class is virtual
//	/*!
//	  TODO Unregistration in the owner subsystem
//	*/
//	virtual ~AbstractSubsystem()
//	{}
//	//! Thread-safely returns subsystem's state
//	inline State state() const
//	{
//		MutexLocker locker(_stateCond.mutex());
//		return _state;
//	}
//	//! Awaits once for subsystem's state to be set to passed one during a passed timeout
//	/*!
//	  \param state State to wait for
//	  \param timeout Timeout of waiting for state
//	  \return Finally inspected state
//	*/
//	/*inline State awaitState(State state, Timeout timeout)
//	{
//
//		MutexLocker locker(_stateCond.mutex());
//		if (_state == state) {
//			return _state;
//		}
//		_stateCond.wait(timeout);
//		return _state;
//	}*/
//	//! Awaits once for subsystem's state to be set
//	/*!
//	  \param timeout Timeout of waiting for state
//	  \return Finally inspected state
//	*/
//	inline State awaitStateChange(Timeout timeout)
//	{
//
//		MutexLocker locker(_stateCond.mutex());
//		_stateCond.wait(timeout);
//		return _state;
//	}
//	//! Asynchronously starting subsystem abstract virtual method to override in descendants
//	/*!
//	  All state manipulations are under this method's control
//	  \return True if the starting process has been successfully launched
//	  TODO Add default implementation, which starts all children subsystems?
//	*/
//	virtual void start() = 0;
//	//! Synchronously stopping subsystem abstract virtual method to override in descendants
//	/*!
//	  All state manipulations are under this method's control
//	  TODO Add default implementation, which stops all children subsystems?
//	*/
//	virtual void stop() = 0;
//	//! Returns state name by state value
//	static const wchar_t * stateName(State state)
//	{
//		switch (state) {
//			case IdlingState:
//				return IdlingStateName;
//				break;
//			case StartingState:
//				return StartingStateName;
//				break;
//			case RunningState:
//				return RunningStateName;
//				break;
//			case StoppingState:
//				return StoppingStateName;
//				break;
//			default:
//				return NotDefinedStateName;
//		}
//	}
//protected:
//	//! Subsystem's state mutex locker helper class
//	/*!
//	  Use this class if you want to proceed some actions during particular state only.
//	*/
//	class StateLocker
//	{
//	public:
//		StateLocker(AbstractSubsystem& subsystem) :
//			_subsystem(subsystem)
//		{
//			_subsystem._stateCond.mutex().lock();
//		}
//		~StateLocker()
//		{
//			_subsystem._stateCond.mutex().unlock();
//		}
//
//		State state() const
//		{
//			return _subsystem._state;
//		}
//		State setState(State newState)
//		{
//			/*State oldState = _subsystem._state;
//			if (newState != oldState) {
//				_subsystem._state = newState;
//				_subsystem._stateCond.wakeAll();
//			}
//			return oldState;*/
//			return _subsystem.setStateUnsafe(newState);
//		}
//		void setState(State oldState, State newState)
//		{
//			/*if (_subsystem._state != oldState) {
//				// TODO Use isl::Exception class
//				throw new std::runtime_error("Invalid subsystem state to switch from");
//			}
//			_subsystem._state = newState;
//			_subsystem._stateCond.wakeAll();*/
//			_subsystem.setStateUnsafe(oldState, newState);
//		}
//		inline State awaitStateChange(Timeout timeout)
//		{
//			_subsystem._stateCond.wait(timeout);
//			return _subsystem._state;
//		}
//	private:
//		StateLocker();
//		StateLocker(const StateLocker&);
//
//		StateLocker& operator=(const StateLocker&);
//		
//		AbstractSubsystem& _subsystem;
//	};
//	//! Thread-safely sets subsystem's state
//	inline State setState(State newState)
//	{
//		/*MutexLocker locker(_stateCond.mutex());
//		_state = newState;
//		_stateCond.wakeAll();*/
//		return setStateUnsafe(newState);
//	}
//	//! Thread-safely sets subsystem's state from one to another
//	/*!
//	  \param oldState Subsystem's state to switch from
//	  \param newState Subsystem's state to switch to
//	*/
//	inline void setState(State oldState, State newState)
//	{
//		/*MutexLocker locker(_stateCond.mutex());
//		if (_state != oldState) {
//			// TODO Use isl::Exception class
//			throw new std::runtime_error("Invalid subsystem state to switch from");
//		}
//		_state = newState;
//		_stateCond.wakeAll();*/
//		setStateUnsafe(oldState, newState);
//	}
//private:
//	AbstractSubsystem();
//	AbstractSubsystem(const AbstractSubsystem&);							// No copy
//
//	AbstractSubsystem& operator=(const AbstractSubsystem&);						// No copy
//
//	State setStateUnsafe(State newState)
//	{
//		State oldState = _state;
//		if (newState != oldState) {
//			_state = newState;
//			_stateCond.wakeAll();
//		}
//		return oldState;
//	}
//	inline void setStateUnsafe(State oldState, State newState)
//	{
//		if (_state != oldState) {
//			// TODO Use isl::Exception class
//			throw new std::runtime_error("Invalid subsystem state to switch from");
//		}
//		_state = newState;
//		_stateCond.wakeAll();
//	}
//
//	AbstractSubsystem * _owner;
//	State _state;
//	mutable WaitCondition _stateCond;
//
//	static const wchar_t NotDefinedStateName[];
//	static const wchar_t IdlingStateName[];
//	static const wchar_t StartingStateName[];
//	static const wchar_t RunningStateName[];
//	static const wchar_t RestartingStateName[];
//	static const wchar_t StoppingStateName[];
//};

} // namespace isl

#endif


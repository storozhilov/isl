#ifndef ISL__ABSTRACT_SUBSYSTEM__HXX
#define ISL__ABSTRACT_SUBSYSTEM__HXX

#include <isl/ReadWriteLock.hxx>
#include <isl/Mutex.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Core.hxx>
#include <isl/Debug.hxx>
#include <string>

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
	//! Thread-safely returns subsystem's state
	inline State state() const
	{
		MutexLocker locker(_stateCond.mutex());
		return _state;
	}
	//! Awaits once for subsystem's state to be set to passed one during a passed timeout
	/*!
	  \param state State to wait for
	  \param timeout Timeout of waiting for state
	  \return Finally inspected state
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
	//! Awaits once for subsystem's state to be set
	/*!
	  \param timeout Timeout of waiting for state
	  \return Finally inspected state
	*/
	inline State awaitState(Timeout timeout)
	{

		MutexLocker locker(_stateCond.mutex());
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
	inline void setState(State newState)
	{
		MutexLocker locker(_stateCond.mutex());
		_state = newState;
		_stateCond.wakeAll();
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

} // namespace isl

#endif


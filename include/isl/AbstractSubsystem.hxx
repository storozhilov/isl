#ifndef ISL__ABSTRACT_SUBSYSTEM__HXX
#define ISL__ABSTRACT_SUBSYSTEM__HXX

#include <isl/ReadWriteLock.hxx>
#include <isl/Mutex.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Enum.hxx>
#include <string>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractSubsystem
 * TODO REMOVE THIS ROCKET SCIENCE!!! :)
------------------------------------------------------------------------------*/

class AbstractSubsystem
{
public:
	AbstractSubsystem(AbstractSubsystem * owner);
	virtual ~AbstractSubsystem();

	//! Base class for subsystem state "Extensible Enum" values
	class AbstractState
	{
	public:
		virtual ~AbstractState() {}
		
		virtual AbstractState * clone() const = 0;
		virtual std::wstring name() const = 0;
	};
	class IdlingState : public AbstractState
	{
	public:
		virtual AbstractState * clone() const { return new IdlingState(*this); }
		virtual std::wstring name() const { return L"Idling"; }
	};
	class StartingState : public AbstractState
	{
	public:
		virtual AbstractState * clone() const { return new StartingState(*this); }
		virtual std::wstring name() const { return L"Starting"; }
	};
	class RunningState : public AbstractState
	{
	public:
		virtual AbstractState * clone() const { return new RunningState(*this); }
		virtual std::wstring name() const { return L"Running"; }
	};
	class StoppingState : public AbstractState
	{
	public:
		virtual AbstractState * clone() const { return new StoppingState(*this); }
		virtual std::wstring name() const { return L"Stopping"; }
	};
	class RestartingState : public AbstractState
	{
	public:
		virtual AbstractState * clone() const { return new RestartingState(*this); }
		virtual std::wstring name() const { return L"Restarting"; }
	};
	//! Subsystem states "Extensible Enum"
	typedef Enum<AbstractState> State;

	//! Base class for subsystem command "Extensible Enum" values
	class AbstractCommand
	{
	public:
		virtual ~AbstractCommand() {}
		
		virtual AbstractCommand * clone() const = 0;
		virtual std::wstring name() const = 0;
	};
	class StartCommand : public AbstractCommand
	{
	public:
		virtual AbstractCommand * clone() const { return new StartCommand(*this); }
		virtual std::wstring name() const { return L"Start"; }
	};
	class StopCommand : public AbstractCommand
	{
	public:
		virtual AbstractCommand * clone() const { return new StopCommand(*this); }
		virtual std::wstring name() const { return L"Stop"; }
	};
	class RestartCommand : public AbstractCommand
	{
	public:
		virtual AbstractCommand * clone() const { return new RestartCommand(*this); }
		virtual std::wstring name() const { return L"Restart"; }
	};
	//! Subsystem commands "Extensible Enum"
	typedef Enum<AbstractCommand> Command;

	inline void start()
	{
		command<StartCommand>();
	}
	inline void stop()
	{
		command<StopCommand>();
	}
	inline void restart()
	{
		command<RestartCommand>();
	}
	inline State state() const
	{
		MutexLocker locker(_stateCond.mutex());
		return _state;
	}
	template <typename S> inline bool isInState() const
	{
		MutexLocker locker(_stateCond.mutex());
		return _state.equals<S>();
	}
	inline bool isInState(const State& state) const
	{
		MutexLocker locker(_stateCond.mutex());
		return (_state == state);
	}
	inline AbstractSubsystem * owner() const
	{
		return _owner;
	}
	inline void command(const Command& cmd)
	{
		MutexLocker locker(_commandMutex);
		onCommand(cmd);
	}
	template <typename Cmd> inline void command()
	{
		command(Command::construct<Cmd>());
	}
	inline State awaitState()
	{
		MutexLocker locker(_stateCond.mutex());
		_stateCond.wait();
		return _state;
	}
	inline State awaitState(Timeout timeout, bool& timeoutExpired)
	{
		MutexLocker locker(_stateCond.mutex());
		timeoutExpired = !_stateCond.wait(timeout);
		return _state;
	}
protected:
	virtual void onCommand(const Command& command);
	virtual void onStartCommand() = 0;
	virtual void onStopCommand() = 0;
	virtual void onRestartCommand()
	{
		onStopCommand();
		onStartCommand();
	}

	template <typename NewState> inline void setState()
	{
		setState(State::construct<NewState>());
	}
	inline void setState(const State& newState)
	{
		MutexLocker locker(_stateCond.mutex());
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
	Mutex _commandMutex;
};

namespace exp
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
	*/
	AbstractSubsystem(AbstractSubsystem * owner) :
		_owner(owner),
		_state(IdlingState),
		_stateCond()
	{}
	//! Virtual destructor cause class is virtual
	virtual ~AbstractSubsystem();
	//! Thread-safely returns subsystem's state
	inline State state() const
	{
		MutexLocker locker(_stateCond.mutex());
		return _state;
	}
	//! Awaits for subsystem's state to be set to passed one during a passed timeout
	/*!
	  \param state State to wait for
	  \param timeout Timeout of waiting
	  \return True if state has been set to the passed one
	*/
	inline bool awaitState(State state, Timeout timeout)
	{

		MutexLocker locker(_stateCond.mutex());
		if (_state == state) {
			return true;
		}
		_stateCond.wait(timeout);
		return _state == state;
	}
	//! Starting subsystem abstract virtual method to override in descendants
	virtual void start() = 0;
	//! Stopping subsystem abstract virtual method to override in descendants
	virtual void stop() = 0;
protected:
	//! Thread-safely sets subsystem's state
	inline void setState(State newState)
	{
		MutexLocker locker(_stateCond.mutex());
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
};

}

} // namespace isl

#endif


#ifndef ISL__TIMER__HXX
#define ISL__TIMER__HXX

#include <isl/Subsystem.hxx>
#include <isl/DateTime.hxx>
#include <map>
#include <list>
#include <vector>

#ifndef ISL__TIMER_ADJUSTMENT_TIMEOUT_SECONDS
#define ISL__TIMER_ADJUSTMENT_TIMEOUT_SECONDS 0				// 0 seconds
#endif
#ifndef ISL__TIMER_ADJUSTMENT_TIMEOUT_NANO_SECONDS
#define ISL__TIMER_ADJUSTMENT_TIMEOUT_NANO_SECONDS 10000000		// 10 milliseconds
#endif

namespace isl
{

//! High-precision timer
/*!
  Timer starts up one thread and executes any of registered tasks during running. You should
  implement Timer::AbstractTask class descendant and register it in timer in order to bring
  it work.
*/
class Timer : public Subsystem
{
public:
	//! Constructs timer
	/*!
	  \param owner Pointer to owner subsystem
	  \param clockTimeout Timer clock timeout
	  \param adjustmentTimeout Timer adjustment timeout which is used in determination of the next unexpired timer tick
	*/
	Timer(Subsystem * owner, const Timeout& clockTimeout = Timeout::defaultTimeout(),
			const Timeout& adjustmentTimeout = Timeout(ISL__TIMER_ADJUSTMENT_TIMEOUT_SECONDS, ISL__TIMER_ADJUSTMENT_TIMEOUT_NANO_SECONDS));
	//! Timestamps container
	typedef std::vector<struct timespec> TimestampContainer;
	//! Abstract timer class task
	class AbstractTask
	{
	public:
		//! Constructs task
		AbstractTask()
		{}
		//! Destructor
		virtual ~AbstractTask()
		{}
	protected:
		//! On timer start event handler
		virtual void onStart(Timer& timer)
		{}
		//! On timer stop event handler
		virtual void onStop(Timer& timer)
		{}
		//! Task execution asbtract virtual method
		/*!
		  \param timer Reference to timer
		  \param expiredTimestamps Container with expired timestamps for the task
		*/
		virtual void execute(Timer& timer, const TimestampContainer& expiredTimestamps) = 0;
	private:
		friend class Timer;
	};

	//! Returns timer clock timeout
	inline Timeout clockTimeout() const
	{
		ReadLocker locker(runtimeParamsRWLock);
		return _clockTimeout;
	}
	//! Sets timer clock timeout
	/*!
	  Timer's restart needed to activate changes.

	  \param newValue New timer clock timeout
	*/
	inline void setClockTimeout(const Timeout& newValue)
	{
		WriteLocker locker(runtimeParamsRWLock);
		_clockTimeout = newValue;
	}
	//! Returns timer adjustment timeout
	inline Timeout adjustmentTimeout() const
	{
		ReadLocker locker(runtimeParamsRWLock);
		return _adjustmentTimeout;
	}
	//! Sets timer adjustment timeout
	/*!
	  Timer's restart needed to activate changes.

	  \param newValue New timer adjustment timeout
	*/
	inline void setAdjustmentTimeout(const Timeout& newValue)
	{
		WriteLocker locker(runtimeParamsRWLock);
		_adjustmentTimeout = newValue;
	}
	//! Registers task in timer
	/*!
	  Timer's restart needed to activate changes.

	  \param task Reference to task to register
	  \param timeout Task execution timeout
	  \return Task ID
	*/
	int registerTask(AbstractTask& task, const Timeout& timeout);
	//! Updates registered task in timer
	/*!
	  Timer's restart needed to activate changes.

	  \param taskId Task ID to update
	  \param newTimeout New task execution timeout
	*/
	void updateTask(int taskId, const Timeout& newTimeout);
	//! Removes registered task in timer
	/*!
	  Timer's restart needed to activate changes.

	  \param taskId Task ID to remove
	*/
	void removeTask(int taskId);
	//! Removes all registered tasks from the timer
	/*!
	  Timer's restart needed to activate changes.
	*/
	void resetTasks();
private:
	Timer();
	Timer(const Timer&);								// No copy

	Timer& operator=(const Timer&);							// No copy

	typedef std::pair<AbstractTask *, Timeout> TaskMapValue;
	typedef std::map<int, TaskMapValue> TaskMap;

	class Thread : public AbstractThread
	{
	public:
		Thread(Timer& timer);
	private:
		Thread();
		Thread(const Thread&);							// No copy

		Thread& operator=(const Thread&);					// No copy

		struct TaskContainerItem
		{
			AbstractTask * taskPtr;
			Timeout timeout;
			struct timespec nextExecutionTimestamp;

			TaskContainerItem(AbstractTask * taskPtr, const Timeout& timeout) :
				taskPtr(taskPtr),
				timeout(timeout),
				nextExecutionTimestamp()
			{}
		};
		typedef std::list<TaskContainerItem> TaskContainer;

		bool hasPendingSignals() const;
		int extractPendingSignal() const;

		virtual void run();

		Timer& _timer;
	};

	Timeout _clockTimeout;
	Timeout _adjustmentTimeout;
	int _lastTaskId;
	TaskMap _taskMap;
	Thread _thread;
};

} // namespace isl

#endif

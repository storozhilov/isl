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
#ifndef ISL__TIMER_MAX_SCHEDULED_TASK_AMOUNT
#define ISL__TIMER_MAX_SCHEDULED_TASK_AMOUNT 1024
#endif

namespace isl
{

//! High-precision timer
/*!
  Timer starts up one thread and executes any of registered tasks during running. You should
  implement Timer::AbstractTask class descendant and register it in timer in order to bring
  it work.

  TODO Reset scheduled tasks map after stop?
*/
class Timer : public Subsystem
{
public:
	//! Constructs timer
	/*!
	  \param owner Pointer to owner subsystem
	  \param clockTimeout Timer clock timeout
	  \param adjustmentTimeout Timer adjustment timeout which is used in determination of the next unexpired timer tick
	  \param maxScheduledTaskAmount Maximum amount of scheduled tasks
	*/
	Timer(Subsystem * owner, const Timeout& clockTimeout = Timeout::defaultTimeout(),
			const Timeout& adjustmentTimeout = Timeout(ISL__TIMER_ADJUSTMENT_TIMEOUT_SECONDS, ISL__TIMER_ADJUSTMENT_TIMEOUT_NANO_SECONDS),
			size_t maxScheduledTaskAmount = ISL__TIMER_MAX_SCHEDULED_TASK_AMOUNT);
	//! Destructor
	virtual ~Timer();
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
		  Exception thrown from this method directs timer to terminate it's execution.

		  \param timer Reference to timer
		  \param lastExpiredTimestamp Last expired timestamp
		  \param expiredTimestamps Expired timestamps amount
		  \param timeout Task execution timeout in timer
		*/
		virtual void execute(Timer& timer, const struct timespec& lastExpiredTimestamp, size_t expiredTimestamps, const Timeout& timeout) = 0;
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
	//! Registers periodic task in timer
	/*!
	  Timer's restart needed to activate changes.

	  \param task Reference to task to register
	  \param timeout Task execution period timeout
	  \return Task ID
	*/
	int registerPeriodicTask(AbstractTask& task, const Timeout& timeout);
	//! Updates registered periodic task in timer
	/*!
	  Timer's restart needed to activate changes.

	  \param taskId Task ID to update
	  \param newTimeout New task execution period timeout
	*/
	void updatePeriodicTask(int taskId, const Timeout& newTimeout);
	//! Removes registered preiodic task in timer
	/*!
	  Timer's restart needed to activate changes.

	  \param taskId Task ID to remove
	*/
	void removePeriodicTask(int taskId);
	//! Removes all registered tasks from the timer
	/*!
	  Timer's restart needed to activate changes.
	*/
	void resetPeriodicTasks();
	//! Schedule a task for single execution
	/*!
	  \param task Reference to task to schedule
	  \param timeout Timeout to wait for the task execution from now
	  \return TRUE if task has been successfully scheduled or FALSE if the scheduled task container has been overflowed
	*/
	bool scheduleTask(AbstractTask& task, const Timeout& timeout);
protected:
	//! After stop event handler redefinition
        inline virtual void afterStop()
	{
		_scheduledTaskMap.clear();
	}
	//! On timer overload event handler
	/*!
	  \param ticksExpired Expired ticks amount (always >= 2 - it's an overload)
	*/
	virtual void onOverload(size_t ticksExpired)
	{}
private:
	Timer();
	Timer(const Timer&);								// No copy

	Timer& operator=(const Timer&);							// No copy

	typedef std::pair<AbstractTask *, Timeout> PeriodicTaskMapValue;
	typedef std::map<int, PeriodicTaskMapValue> PeriodicTaskMap;
	typedef std::multimap<struct timespec, AbstractTask *, TimeSpecComp> ScheduledTaskMap;

	class Thread : public AbstractThread
	{
	public:
		Thread(Timer& timer);
	private:
		Thread();
		Thread(const Thread&);							// No copy

		Thread& operator=(const Thread&);					// No copy

		struct PeriodicTaskContainerItem
		{
			AbstractTask * taskPtr;
			Timeout timeout;
			struct timespec nextExecutionTimestamp;

			PeriodicTaskContainerItem(AbstractTask * taskPtr, const Timeout& timeout) :
				taskPtr(taskPtr),
				timeout(timeout),
				nextExecutionTimestamp()
			{}
		};
		typedef std::list<PeriodicTaskContainerItem> PeriodicTaskContainer;

		bool hasPendingSignals() const;
		int extractPendingSignal() const;

		virtual void run();

		Timer& _timer;
	};

	Timeout _clockTimeout;
	Timeout _adjustmentTimeout;
	size_t _maxScheduledTaskAmount;
	int _lastPeriodicTaskId;
	PeriodicTaskMap _periodicTaskMap;
	ScheduledTaskMap _scheduledTaskMap;
	Thread _thread;
};

} // namespace isl

#endif

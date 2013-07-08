#ifndef ISL__TIMER__HXX
#define ISL__TIMER__HXX

#include <isl/Subsystem.hxx>
#include <isl/DateTime.hxx>
#include <isl/TimeSpec.hxx>
#include <isl/Timestamp.hxx>
#include <map>
#include <list>
#include <vector>

#ifndef ISL__TIMER_DEFAULT_MAX_SCHEDULED_TASKS_AMOUNT
#define ISL__TIMER_DEFAULT_MAX_SCHEDULED_TASKS_AMOUNT 1024
#endif

namespace isl
{

//! High-precision timer
/*!
  Timer executes tasks in it's thread. A task could be:

  - <strong>Periodic</strong>, which is to be registered during timer idling only and executed periodically;
  - <strong>Scheduled</strong>, which is to be executed only once.

  To implement a periodic task just make a subclass of Timer::AbstractPeriodicTask class and register it
  using Timer::registerPeriodicTask() method. To implement a scheduled task just make a subclass of
  Timer::AbstractScheduledTask class and schedule it using Timer::scheduleTask() method.
*/
class Timer : public Subsystem
{
public:
	enum Constants {
		DefaultMaxScheduledTasksAmount = ISL__TIMER_DEFAULT_MAX_SCHEDULED_TASKS_AMOUNT
	};
	//! Constructs timer
	/*!
	  \param owner Pointer to owner subsystem
	  \param clockTimeout Timer clock timeout
	  \param maxScheduledTasksAmount Maximum amount of scheduled tasks
	*/
	Timer(Subsystem * owner, const Timeout& clockTimeout = Timeout::defaultTimeout(),
			size_t maxScheduledTasksAmount = DefaultMaxScheduledTasksAmount);
	//! Destructor
	virtual ~Timer();
	//! Abstract periodic task for the timer
	class AbstractPeriodicTask
	{
	public:
		//! Constructs task
		AbstractPeriodicTask()
		{}
		//! Destructor
		virtual ~AbstractPeriodicTask()
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
		  \param lastExpiredTimestamp Last expired timestamp for the task execution
		  \param expiredTimestamps Expired timestamps amount
		  \param timeout Task execution timeout in timer
		*/
		virtual void execute(Timer& timer, const Timestamp& lastExpiredTimestamp, size_t expiredTimestamps, const Timeout& timeout) = 0;
	private:
		friend class Timer;
	};
	//! Abstract scheduled task for the timer
	class AbstractScheduledTask
	{
	public:
		//! Constructs task
		AbstractScheduledTask()
		{}
		//! Destructor
		virtual ~AbstractScheduledTask()
		{}
	protected:
		//! Task execution asbtract virtual method
		/*!
		  \param timer Reference to timer
		  \param timestamp Task execution timestamp in timer
		*/
		virtual void execute(Timer& timer, const Timestamp& timestamp) = 0;
	private:
		friend class Timer;
	};

	//! Registers periodic task in timer
	/*!
	  \param task Reference to task to register
	  \param timeout Task execution period timeout
	  \return Task ID

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	int registerPeriodicTask(AbstractPeriodicTask& task, const Timeout& timeout);
	//! Updates registered periodic task in timer
	/*!
	  \param taskId Task ID to update
	  \param newTimeout New task execution period timeout

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	void updatePeriodicTask(int taskId, const Timeout& newTimeout);
	//! Removes registered preiodic task in timer
	/*!
	  \param taskId Task ID to remove

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	void removePeriodicTask(int taskId);
	//! Removes all registered tasks from the timer
	/*!
	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	void resetPeriodicTasks();
	//! Schedule a task for single execution
	/*!
	  \param task Reference to task to schedule
	  \param limit Limit timestamp task execution should be at
	  \return TRUE if task has been successfully scheduled or FALSE if the scheduled task container has been overflowed
	  \note Thread-safe. The task will be executed even if the limit timestamp has been passed!
	*/
	bool scheduleTask(AbstractScheduledTask& task, const Timestamp& limit);
	//! Schedule a task for single execution
	/*!
	  \param task Reference to task to schedule
	  \param timeout Timeout to wait for the task execution from now
	  \return TRUE if task has been successfully scheduled or FALSE if the scheduled task container has been overflowed
	  \note Thread-safe. The task will be executed even if zero timeout has been supplied!
	*/
	inline bool scheduleTask(AbstractScheduledTask& task, const Timeout& timeout)
	{
		return scheduleTask(task, Timestamp::limit(timeout));
	}

	//! Starting subsystem virtual method
	/*!
	  \note Thread-unsafe
	*/
	virtual void start();
	//! Stopping subsystem and awaiting for it's termination virtual method
	/*!
	  \note Thread-unsafe
	*/
	virtual void stop();
protected:
	class TimerThread : public RequesterThread
	{
	public:
		TimerThread(Timer& timer);

		//! On start event handler
		/*!
		  Default implementation does nothing and returns TRUE.
		  \return TRUE if to continue thread execution
		*/
		virtual bool onStart();
		//! Doing the work virtual method
		/*!
		  \param prevTickTimestamp Previous tick timestamp
		  \param nextTickTimestamp Next tick timestamp
		  \param ticksExpired Amount of expired ticks - if > 1, then an overload has occured
		  \return TRUE if to continue thread execution
		*/
		virtual bool doLoad(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired);
		//! On overload event handler
		/*!
		  \param prevTickTimestamp Previous tick timestamp
		  \param nextTickTimestamp Next tick timestamp
		  \param Amount of expired ticks - always > 2
		  \return TRUE if to continue thread execution
		*/
		virtual bool onOverload(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
		{
			return _timer.onOverload(prevTickTimestamp, nextTickTimestamp, ticksExpired);
		}
		//! On thread request event handler
		/*!
		  \param pendingRequest Constant reference to pending resuest to process
		*/
		virtual void onRequest(const ThreadRequesterType::PendingRequest& pendingRequest)
		{
			_timer.onRequest(pendingRequest);
		}
		//! On stop event handler
		virtual void onStop();
	private:
		TimerThread();
		TimerThread(const TimerThread&);							// No copy

		TimerThread& operator=(const TimerThread&);						// No copy

		Timer& _timer;
	};

	//! On timer overload event handler
	/*!
	  Default implementation does nothing and returns TRUE.
	  \param prevTickTimestamp Previous tick timestamp
	  \param nextTickTimestamp Next tick timestamp
	  \param Amount of expired ticks - always > 2
	  \return TRUE if to continue thread execution
	*/
	virtual bool onOverload(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
	{
		return true;
	}
	//! On thread request event handler
	/*!
	  Default implementation does nothing.
	  \param pendingRequest Constant reference to pending resuest to process
	*/
	virtual void onRequest(const ThreadRequesterType::PendingRequest& pendingRequest)
	{}
	//! Timer thread creation factory method
	/*!
	 * \returns A pointer to new timer thread object
	 */
	virtual TimerThread * createThread()
	{
		return new TimerThread(*this);
	}
private:
	Timer();
	Timer(const Timer&);								// No copy

	Timer& operator=(const Timer&);							// No copy

	struct PeriodicTaskMapValue
	{
		AbstractPeriodicTask * taskPtr;
		Timeout timeout;
		Timestamp nextExecutionTimestamp;

		PeriodicTaskMapValue(AbstractPeriodicTask * taskPtr, const Timeout& timeout) :
			taskPtr(taskPtr),
			timeout(timeout),
			nextExecutionTimestamp()
		{}
	};
	typedef std::map<int, PeriodicTaskMapValue> PeriodicTasksMap;
	typedef std::multimap<Timestamp, AbstractScheduledTask *> ScheduledTasksMap;

	size_t _maxScheduledTaskAmount;
	int _lastPeriodicTaskId;
	PeriodicTasksMap _periodicTasksMap;
	ReadWriteLock _scheduledTasksRWLock;
	ScheduledTasksMap _scheduledTasksMap;
	std::auto_ptr<TimerThread> _threadAutoPtr;
};

} // namespace isl

#endif

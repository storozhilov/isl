#include <isl/Timer.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>
#include <isl/Error.hxx>

namespace isl
{

//------------------------------------------------------------------------------
// Timer
//------------------------------------------------------------------------------

Timer::Timer(Subsystem * owner, const Timeout& clockTimeout, size_t maxScheduledTaskAmount) :
	Subsystem(owner, clockTimeout),
	_maxScheduledTaskAmount(maxScheduledTaskAmount),
	_lastPeriodicTaskId(0),
	_periodicTasksMap(),
	_scheduledTasksRWLock(),
	_scheduledTasksMap(),
	_threadAutoPtr()
{}

Timer::~Timer()
{}

int Timer::registerPeriodicTask(AbstractPeriodicTask& task, const Timeout& timeout)
{
	if (timeout.isZero()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Zero timeout is not permitted for periodic task"));
	}
	_periodicTasksMap.insert(PeriodicTasksMap::value_type(++_lastPeriodicTaskId, PeriodicTaskMapValue(&task, timeout)));
	return _lastPeriodicTaskId;
}

void Timer::updatePeriodicTask(int taskId, const Timeout& newTimeout)
{
	if (newTimeout.isZero()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Zero timeout is not permitted for periodic task"));
	}
	PeriodicTasksMap::iterator pos = _periodicTasksMap.find(taskId);
	if (pos == _periodicTasksMap.end()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Task (id = ") << taskId << ") not found in timer");
	} else {
		pos->second.timeout = newTimeout;
	}
}

void Timer::removePeriodicTask(int taskId)
{
	PeriodicTasksMap::iterator pos = _periodicTasksMap.find(taskId);
	if (pos == _periodicTasksMap.end()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Task (id = ") << taskId << ") not found in timer");
	} else {
		_periodicTasksMap.erase(pos);
	}
}

void Timer::resetPeriodicTasks()
{
	_periodicTasksMap.clear();
}

bool Timer::scheduleTask(AbstractScheduledTask& task, const Timestamp& timestamp)
{
	WriteLocker locker(_scheduledTasksRWLock);
	if (_scheduledTasksMap.size() >= _maxScheduledTaskAmount) {
		return false;
	}
	_scheduledTasksMap.insert(ScheduledTasksMap::value_type(timestamp, &task));
	return true;
}

void Timer::start()
{
	_threadAutoPtr.reset(createThread());
	Subsystem::start();
}

void Timer::stop()
{
	Subsystem::stop();
	_threadAutoPtr.reset();
}

//------------------------------------------------------------------------------
// Timer::TimerThread
//------------------------------------------------------------------------------

Timer::TimerThread::TimerThread(Timer& timer) :
	RequesterThread(timer),
	_timer(timer)
{}

void Timer::TimerThread::onStart()
{
	RequesterThread::onStart();
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Timer thread has been started"));
	// Calling onStart event handler for any periodic task
	for (PeriodicTasksMap::iterator i = _timer._periodicTasksMap.begin(); i != _timer._periodicTasksMap.end(); ++i) {
		i->second.taskPtr->onStart(_timer);
		i->second.nextExecutionTimestamp.reset();
	}
}

void Timer::TimerThread::doLoad(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
{
	// Executing periodic tasks, which are to be expired until next tick timestamp
	for (PeriodicTasksMap::iterator i = _timer._periodicTasksMap.begin(); i != _timer._periodicTasksMap.end(); ++i) {
		size_t expiredTimestamps = 0;
		Timestamp lastExpiredTimestamp;
		bool shouldExecute = false;
		if (i->second.nextExecutionTimestamp.isZero()) {
			// No executions before
			i->second.nextExecutionTimestamp = Timestamp::now() + i->second.timeout;
			shouldExecute = true;
		} else {
			while (i->second.nextExecutionTimestamp < nextTickTimestamp) {
				lastExpiredTimestamp = i->second.nextExecutionTimestamp;
				++expiredTimestamps;
				i->second.nextExecutionTimestamp += i->second.timeout;
			}
			if (expiredTimestamps > 0) {
				shouldExecute = true;
			}
		}
		if (shouldExecute) {
			i->second.taskPtr->execute(_timer, lastExpiredTimestamp, expiredTimestamps, i->second.timeout);
		}
	}
	// Extracting scheduled tasks, which are to be expired until next tick timestamp
	ScheduledTasksMap expiredScheduledTasks;
	{
		ReadLocker locker(_timer._scheduledTasksRWLock);
		ScheduledTasksMap::iterator pos = _timer._scheduledTasksMap.upper_bound(nextTickTimestamp);
		expiredScheduledTasks.insert(_timer._scheduledTasksMap.begin(), pos);
		_timer._scheduledTasksMap.erase(_timer._scheduledTasksMap.begin(), pos);
	}
	// Executing expired scheduled tasks
	for (ScheduledTasksMap::iterator i = expiredScheduledTasks.begin(); i != expiredScheduledTasks.end(); ++i) {
		i->second->execute(_timer, i->first);
	}
}

void Timer::TimerThread::onStop()
{
	// Calling onStop event handler for any periodic task
	for (PeriodicTasksMap::iterator i = _timer._periodicTasksMap.begin(); i != _timer._periodicTasksMap.end(); ++i) {
		i->second.taskPtr->onStop(_timer);
	}
	RequesterThread::onStop();
}

} // namespace isl

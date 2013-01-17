#include <isl/Timer.hxx>
#include <isl/common.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>
#include <isl/Error.hxx>

namespace isl
{

//------------------------------------------------------------------------------
// Timer
//------------------------------------------------------------------------------

Timer::Timer(Subsystem * owner, const Timeout& clockTimeout, size_t maxScheduledTaskAmount) :
	StateSetSubsystem(owner, clockTimeout),
	_maxScheduledTaskAmount(maxScheduledTaskAmount),
	_lastPeriodicTaskId(0),
	_periodicTasksMap(),
	_scheduledTasksMap(),
	_thread(*this)
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
		//std::ostringstream msg;
		//msg << "Task (id = " << taskId << ") not found in timer";
		//debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Task (id = ") << taskId << ") not found in timer");
	} else {
		pos->second.timeout = newTimeout;
	}
}

void Timer::removePeriodicTask(int taskId)
{
	PeriodicTasksMap::iterator pos = _periodicTasksMap.find(taskId);
	if (pos == _periodicTasksMap.end()) {
		//std::ostringstream msg;
		//msg << "Task (id = " << taskId << ") not found in timer";
		//debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Task (id = ") << taskId << ") not found in timer");
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
	MutexLocker locker(stateSet().cond().mutex());
	if (_scheduledTasksMap.size() >= _maxScheduledTaskAmount) {
		return false;
	}
	_scheduledTasksMap.insert(ScheduledTasksMap::value_type(timestamp, &task));
	return true;
}

//------------------------------------------------------------------------------
// Timer::TimerThread
//------------------------------------------------------------------------------

Timer::TimerThread::TimerThread(Timer& timer) :
	AbstractThread(timer),
	_timer(timer)
{}

void Timer::TimerThread::run()
{
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Timer thread has been started"));
	// Calling onStart event handler for any periodic task
	for (PeriodicTasksMap::iterator i = _timer._periodicTasksMap.begin(); i != _timer._periodicTasksMap.end(); ++i) {
		i->second.taskPtr->onStart(_timer);
	}
	// Setting next execution timestamp for all tasks to now
	Timestamp prevTickTimestamp = Timestamp::now();
	for (PeriodicTasksMap::iterator i = _timer._periodicTasksMap.begin(); i != _timer._periodicTasksMap.end(); ++i) {
		i->second.nextExecutionTimestamp = prevTickTimestamp;
	}
	// Timer's main loop
	while (true) {
		// Identifying next tick timestamp
		Timestamp nextTickTimestamp = prevTickTimestamp;
		size_t ticksExpired = 0;
		Timestamp nowTimestamp = Timestamp::now();
		while (nextTickTimestamp <= nowTimestamp) {
			++ticksExpired;
			nextTickTimestamp += _timer.clockTimeout();
		}
		// Reporting if the timer overload has been detected before awaiting
		if (ticksExpired > 1) {
			_timer.onOverload(ticksExpired);
		}
		// Executing periodic tasks, which are to be expired until next tick timestamp
		for (PeriodicTasksMap::iterator i = _timer._periodicTasksMap.begin(); i != _timer._periodicTasksMap.end(); ++i) {
			size_t expiredTimestamps = 0;
			Timestamp lastExpiredTimestamp;
			while (i->second.nextExecutionTimestamp < nextTickTimestamp) {
				lastExpiredTimestamp = i->second.nextExecutionTimestamp;
				++expiredTimestamps;
				i->second.nextExecutionTimestamp += i->second.timeout;
			}
			if (expiredTimestamps > 0) {
				i->second.taskPtr->execute(_timer, lastExpiredTimestamp, expiredTimestamps, i->second.timeout);
			}
		}
		// Extracting scheduled tasks, which are to be expired until next tick timestamp
		ScheduledTasksMap expiredScheduledTasks;
		{
			MutexLocker locker(_timer.stateSet().cond().mutex());
			ScheduledTasksMap::iterator pos = _timer._scheduledTasksMap.upper_bound(nextTickTimestamp);
			expiredScheduledTasks.insert(_timer._scheduledTasksMap.begin(), pos);
			_timer._scheduledTasksMap.erase(_timer._scheduledTasksMap.begin(), pos);
		}
		// Executing expired scheduled tasks
		for (ScheduledTasksMap::iterator i = expiredScheduledTasks.begin(); i != expiredScheduledTasks.end(); ++i) {
			i->second->execute(_timer, i->first);
		}
		// Awaiting for the next tick
		if (awaitTermination(nextTickTimestamp)) {
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Timer thread termination detected -> exiting from the timer thread"));
			break;
		}
		// Switching to the next tick
		prevTickTimestamp = nextTickTimestamp;
	}
	// Calling onStop event handler for any periodic task
	for (PeriodicTasksMap::iterator i = _timer._periodicTasksMap.begin(); i != _timer._periodicTasksMap.end(); ++i) {
		i->second.taskPtr->onStop(_timer);
	}
}

} // namespace isl

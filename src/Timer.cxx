#include <isl/Timer.hxx>
#include <isl/common.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>

namespace isl
{

//------------------------------------------------------------------------------
// Timer
//------------------------------------------------------------------------------

Timer::Timer(Subsystem * owner, const Timeout& clockTimeout, const Timeout& adjustmentTimeout,
			size_t maxScheduledTaskAmount) :
	Subsystem(owner),
	_clockTimeout(clockTimeout),
	_adjustmentTimeout(adjustmentTimeout),
	_maxScheduledTaskAmount(maxScheduledTaskAmount),
	_lastPeriodicTaskId(0),
	_periodicTaskMap(),
	_scheduledTaskMap(),
	_thread(*this)
{}

Timer::~Timer()
{}

int Timer::registerPeriodicTask(AbstractTask& task, const Timeout& timeout)
{
	if (timeout.isZero()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Zero timeout is not permitted"));
	}
	WriteLocker locker(runtimeParamsRWLock);
	_periodicTaskMap.insert(PeriodicTaskMap::value_type(++_lastPeriodicTaskId, PeriodicTaskMapValue(&task, timeout)));
	return _lastPeriodicTaskId;
}

void Timer::updatePeriodicTask(int taskId, const Timeout& newTimeout)
{
	if (newTimeout.isZero()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Zero timeout is not permitted"));
	}
	bool taskNotFound = false;
	{
		WriteLocker locker(runtimeParamsRWLock);
		PeriodicTaskMap::iterator pos = _periodicTaskMap.find(taskId);
		if (pos == _periodicTaskMap.end()) {
			taskNotFound = true;
		} else {
			pos->second.second = newTimeout;
		}
	}
	if (taskNotFound) {
		std::ostringstream msg;
		msg << "Task (id = " << taskId << ") not found in timer";
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
	}
}

void Timer::removePeriodicTask(int taskId)
{
	bool taskNotFound = false;
	{
		WriteLocker locker(runtimeParamsRWLock);
		PeriodicTaskMap::iterator pos = _periodicTaskMap.find(taskId);
		if (pos == _periodicTaskMap.end()) {
			taskNotFound = true;
		} else {
			_periodicTaskMap.erase(pos);
		}
	}
	if (taskNotFound) {
		std::ostringstream msg;
		msg << "Task (id = " << taskId << ") not found in timer";
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
	}
}

void Timer::resetPeriodicTasks()
{
	WriteLocker locker(runtimeParamsRWLock);
	_periodicTaskMap.clear();
}

bool Timer::scheduleTask(AbstractTask& task, const Timeout& timeout)
{
	WriteLocker locker(runtimeParamsRWLock);
	if (_scheduledTaskMap.size() >= _maxScheduledTaskAmount) {
		return false;
	}
	_scheduledTaskMap.insert(ScheduledTaskMap::value_type(timeout.limit(), &task));
	return true;
}

//------------------------------------------------------------------------------
// Timer::Thread
//------------------------------------------------------------------------------

Timer::Thread::Thread(Timer& timer) :
	AbstractThread(timer),
	_timer(timer)
{}

void Timer::Thread::run()
{
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Timer thread has been started"));
	try {
		Timeout clockTimeout;
		Timeout adjustmentTimeout;
		PeriodicTaskContainer periodicTasks;
		{
			ReadLocker locker(_timer.runtimeParamsRWLock);
			clockTimeout = _timer._clockTimeout;
			adjustmentTimeout = _timer._adjustmentTimeout;
			for (PeriodicTaskMap::const_iterator i = _timer._periodicTaskMap.begin(); i != _timer._periodicTaskMap.end(); ++i) {
				periodicTasks.push_back(PeriodicTaskContainerItem(i->second.first, i->second.second));
			}
		}
		// Calling onStart event handler for any periodic task
		for (PeriodicTaskContainer::iterator i = periodicTasks.begin(); i != periodicTasks.end(); ++i) {
			i->taskPtr->onStart(_timer);
		}
		struct timespec lastTickTimestamp = BasicDateTime::nowTimeSpec();
		// Execute any periodic task for the first time
		for (PeriodicTaskContainer::iterator i = periodicTasks.begin(); i != periodicTasks.end(); ++i) {
			i->taskPtr->execute(_timer, lastTickTimestamp, 1, i->timeout.timeSpec());
			i->nextExecutionTimestamp = lastTickTimestamp + i->timeout.timeSpec();
		}
		// Timer's main loop
		while (true) {
			// Identifying next tick timestamp
			size_t ticksExpired = 0;
			struct timespec nextTickTimestamp = lastTickTimestamp;
			do {
				++ticksExpired;
				nextTickTimestamp += clockTimeout.timeSpec();
				// TODO Check out too big delay?
			} while (nextTickTimestamp - adjustmentTimeout.timeSpec() < BasicDateTime::nowTimeSpec());
			// Waiting until new tick is coming or termination detected
			if (awaitShouldTerminate(Timeout::leftToLimit(nextTickTimestamp))) {
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Timer thread termination detected -> exiting from timer thread"));
				break;
			}
			// Reporting if the timer overload has been detected before awaiting
			if (ticksExpired > 1) {
				_timer.onOverload(ticksExpired);
			}
			// Executing expired periodic tasks
			for (PeriodicTaskContainer::iterator i = periodicTasks.begin(); i != periodicTasks.end(); ++i) {
				size_t expiredTimestamps = 0;
				struct timespec lastExpiredTimestamp;
				while (i->nextExecutionTimestamp < nextTickTimestamp) {
					lastExpiredTimestamp = i->nextExecutionTimestamp;
					++expiredTimestamps;
					i->nextExecutionTimestamp += i->timeout.timeSpec();
				}
				if (expiredTimestamps > 0) {
					i->taskPtr->execute(_timer, lastExpiredTimestamp, expiredTimestamps, i->timeout);
				}
			}
			// Executing expired scheduled tasks
			ScheduledTaskMap expiredScheduledTasks;
			{
				ReadLocker locker(_timer.runtimeParamsRWLock);
				ScheduledTaskMap::iterator pos = _timer._scheduledTaskMap.upper_bound(nextTickTimestamp);
				expiredScheduledTasks.insert(_timer._scheduledTaskMap.begin(), pos);
				_timer._scheduledTaskMap.erase(_timer._scheduledTaskMap.begin(), pos);
			}
			for (ScheduledTaskMap::iterator i = expiredScheduledTasks.begin(); i != expiredScheduledTasks.end(); ++i) {
				i->second->execute(_timer, i->first, 1, Timeout());
			}
			// Switching to the next tick
			lastTickTimestamp = nextTickTimestamp;
		}
		// Calling onStop event handler for any periodic task
		for (PeriodicTaskContainer::iterator i = periodicTasks.begin(); i != periodicTasks.end(); ++i) {
			i->taskPtr->onStop(_timer);
		}
	} catch (std::exception& e) {
		errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Executing timer thread error -> exiting from timer thread"));
	} catch (...) {
		errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Executing timer thread unknown error -> exiting from timer thread"));
	}
}

} // namespace isl

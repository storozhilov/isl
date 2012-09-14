#include <isl/Timer.hxx>
#include <isl/common.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>

namespace isl
{

//------------------------------------------------------------------------------
// Timer
//------------------------------------------------------------------------------

Timer::Timer(Subsystem * owner, const Timeout& clockTimeout, const Timeout& adjustmentTimeout) :
	Subsystem(owner),
	_clockTimeout(clockTimeout),
	_adjustmentTimeout(adjustmentTimeout),
	_lastTaskId(0),
	_taskMap(),
	_thread(*this)
{}

int Timer::registerTask(AbstractTask& task, const Timeout& timeout)
{
	if (timeout.isZero()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Zero timeout is not permitted"));
	}
	WriteLocker locker(runtimeParamsRWLock);
	_taskMap.insert(TaskMap::value_type(++_lastTaskId, TaskMapValue(&task, timeout)));
	return _lastTaskId;
}

void Timer::updateTask(int taskId, const Timeout& newTimeout)
{
	if (newTimeout.isZero()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Zero timeout is not permitted"));
	}
	bool taskNotFound = false;
	{
		WriteLocker locker(runtimeParamsRWLock);
		TaskMap::iterator pos = _taskMap.find(taskId);
		if (pos == _taskMap.end()) {
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

void Timer::removeTask(int taskId)
{
	bool taskNotFound = false;
	{
		WriteLocker locker(runtimeParamsRWLock);
		TaskMap::iterator pos = _taskMap.find(taskId);
		if (pos == _taskMap.end()) {
			taskNotFound = true;
		} else {
			_taskMap.erase(pos);
		}
	}
	if (taskNotFound) {
		std::ostringstream msg;
		msg << "Task (id = " << taskId << ") not found in timer";
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
	}
}

void Timer::resetTasks()
{
	WriteLocker locker(runtimeParamsRWLock);
	_taskMap.clear();
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
		TaskContainer taskContainer;
		{
			ReadLocker locker(_timer.runtimeParamsRWLock);
			clockTimeout = _timer._clockTimeout;
			adjustmentTimeout = _timer._adjustmentTimeout;
			for (TaskMap::const_iterator i = _timer._taskMap.begin(); i != _timer._taskMap.end(); ++i) {
				taskContainer.push_back(TaskContainerItem(i->second.first, i->second.second));
			}
		}
		// Calling onStart event handler for any task
		for (TaskContainer::iterator i = taskContainer.begin(); i != taskContainer.end(); ++i) {
			i->taskPtr->onStart(_timer);
		}
		struct timespec lastTickTimestamp = now();
		// Execute any task for the first time
		TimestampContainer tc(1, lastTickTimestamp);
		for (TaskContainer::iterator i = taskContainer.begin(); i != taskContainer.end(); ++i) {
			i->taskPtr->execute(_timer, tc);
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
				// TODO Check out too big delay
			} while (nextTickTimestamp - adjustmentTimeout.timeSpec() < now());
			// Waiting until new tick is coming or termination detected
			if (awaitShouldTerminate(Timeout::leftToLimit(nextTickTimestamp))) {
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Timer thread termination detected -> exiting from timer thread"));
				break;
			}
			// Reporting if the timer overload has been detected
			if (ticksExpired > 1) {
				std::ostringstream msg;
				msg << "Timer overload has been detected: " << ticksExpired << " ticks expired";
				warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
			// Executing tasks if needed
			for (TaskContainer::iterator i = taskContainer.begin(); i != taskContainer.end(); ++i) {
				TimestampContainer expiredTimestamps;
				while (i->nextExecutionTimestamp < nextTickTimestamp) {
					expiredTimestamps.push_back(i->nextExecutionTimestamp);
					i->nextExecutionTimestamp += i->timeout.timeSpec();
				}
				if (!expiredTimestamps.empty()) {
					i->taskPtr->execute(_timer, expiredTimestamps);
				}
				lastTickTimestamp = nextTickTimestamp;
			}
		}
		// Calling onStop event handler for any task
		for (TaskContainer::iterator i = taskContainer.begin(); i != taskContainer.end(); ++i) {
			i->taskPtr->onStop(_timer);
		}
	} catch (std::exception& e) {
		errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Executing timer thread error -> exiting from timer thread"));
	} catch (...) {
		errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Executing timer thread unknown error -> exiting from timer thread"));
	}
}

} // namespace isl

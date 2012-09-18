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

Timer::~Timer()
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
		struct timespec lastTickTimestamp = BasicDateTime::nowTimeSpec();
		// Execute any task for the first time
		for (TaskContainer::iterator i = taskContainer.begin(); i != taskContainer.end(); ++i) {
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
			// Executing expired tasks
			for (TaskContainer::iterator i = taskContainer.begin(); i != taskContainer.end(); ++i) {
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
			// Switching to the next tick
			lastTickTimestamp = nextTickTimestamp;
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

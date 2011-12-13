#include <isl/TaskDispatcher.hxx>
#include <isl/Mutex.hxx>
#include <isl/Core.hxx>
#include <isl/Exception.hxx>
#include <isl/VariantFormatter.hxx>
#include <exception>
#include <sstream>
#include <memory>

namespace isl
{

/*------------------------------------------------------------------------------
 * TaskDispatcher
------------------------------------------------------------------------------*/

TaskDispatcher::TaskDispatcher(AbstractSubsystem * owner, unsigned int workersCount, unsigned int maxTaskQueueOverflowSize) :
	AbstractSubsystem(owner),
	_workersCount(workersCount),
	_taskCond(),
	_awaitingWorkersCount(0),
	_maxTaskQueueOverflowSize(maxTaskQueueOverflowSize),
	_tasks(),
	_workers()
{
	for (int i = 0; i < workersCount; ++i) {
		_workers.push_back(createWorker(i));
	}
}

TaskDispatcher::~TaskDispatcher()
{
	for (Workers::iterator i = _workers.begin(); i != _workers.end(); ++i) {
		delete (*i);
	}
	for (Tasks::iterator i = _tasks.begin(); i != _tasks.end(); ++i) {
		delete (*i);
	}
}

bool TaskDispatcher::perform(AbstractTask * task)
{
	unsigned int awaitingWorkersCount;
	unsigned int taskQueueSize;
	bool taskPerformed = false;
	{
		MutexLocker locker(_taskCond.mutex());
		awaitingWorkersCount = _awaitingWorkersCount;
		taskQueueSize = _tasks.size();
		if ((awaitingWorkersCount + _maxTaskQueueOverflowSize) >= (taskQueueSize + 1)) {
			//_tasks.push_back(task);
			_tasks.push_front(task);
			_taskCond.wakeOne();
			taskPerformed = true;
		}
	}
	std::wostringstream oss;
	oss << L"Total workers: " << _workers.size() << ", workers awaiting: " << awaitingWorkersCount << L", tasks in pool: " << (taskQueueSize + 1) <<
		L", max task overflow: "  << _maxTaskQueueOverflowSize << L", detected tasks overflow: " <<
		((awaitingWorkersCount >= (taskQueueSize + 1)) ? 0 : taskQueueSize + 1 - awaitingWorkersCount);
	if (taskPerformed) {
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
	} else {
		Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
	}
	// TODO The following is thrown "Variant type is not supported..." exception
	/*VariantWFormatter fmt(L"Workers awaiting: $0, tasks in pool: $1, available overflow: $2, tasks overflow: $3");
	fmt.arg(awaitingWorkersCount).arg(taskQueueSize + 1).arg(_maxTaskQueueOverflowSize).arg((awaitingWorkersCount >= (taskQueueSize + 1)) ? 0 : taskQueueSize + 1 - awaitingWorkersCount);
	if (taskPerformed) {
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, fmt.compose()));
	} else {
		Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, fmt.compose()));
	}*/
	return taskPerformed;
}

bool TaskDispatcher::perform(const TaskList& taskList)
{
	unsigned int tasksCount = taskList.size();
	if (tasksCount <= 0) {
		return true;
	}
	unsigned int awaitingWorkersCount;
	unsigned int taskQueueSize;
	bool tasksPerformed = false;
	{
		MutexLocker locker(_taskCond.mutex());
		awaitingWorkersCount = _awaitingWorkersCount;
		taskQueueSize = _tasks.size();
		if ((awaitingWorkersCount + _maxTaskQueueOverflowSize) >= (taskQueueSize + tasksCount)) {
			for (TaskList::const_iterator i = taskList.begin(); i != taskList.end(); ++i) {
				//_tasks.push_back(*i);
				_tasks.push_front(*i);
			}
			_taskCond.wakeAll();
			tasksPerformed = true;
		}
	}
	std::wostringstream oss;
	oss << L"Total workers: " << _workers.size() << ", workers awaiting: " << awaitingWorkersCount << L", tasks to execute: " << tasksCount <<
		L" tasks in pool: " << (taskQueueSize + tasksCount) << L", max task overflow: "  << _maxTaskQueueOverflowSize << L", detected tasks overflow: " <<
		((awaitingWorkersCount >= (taskQueueSize + tasksCount)) ? 0 : taskQueueSize + 1 - awaitingWorkersCount);
	if (tasksPerformed) {
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
	} else {
		Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
	}
	return tasksPerformed;
}

bool TaskDispatcher::start()
{
	setState(IdlingState, StartingState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Starting subsystem"));
	for (Workers::iterator i = _workers.begin(); i != _workers.end(); ++i) {
		(*i)->start();
	}
	// TODO Maybe to wait until all workers have been started and ready to work?
	setState(StartingState, RunningState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been started"));
	return true;
}

void TaskDispatcher::stop()
{
	setState(StoppingState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Stopping subsystem"));
	{
		MutexLocker locker(_taskCond.mutex());
		_taskCond.wakeAll();
	}
	for (Workers::iterator i = _workers.begin(); i != _workers.end(); ++i) {
		(*i)->join();
	}
	setState(IdlingState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been stopped"));
}

bool TaskDispatcher::restart()
{
	setState(StoppingState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Stopping subsystem during restart"));
	{
		MutexLocker locker(_taskCond.mutex());
		_taskCond.wakeAll();
	}
	for (Workers::iterator i = _workers.begin(); i != _workers.end(); ++i) {
		(*i)->join();
	}
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been stopped during restart"));
	setState(StoppingState, StartingState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Starting subsystem during restart"));
	for (Workers::iterator i = _workers.begin(); i != _workers.end(); ++i) {
		(*i)->start();
	}
	// TODO Maybe to wait until all workers have been started and ready to work?
	setState(StartingState, RunningState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been started during restart"));
	return true;
}

Worker * TaskDispatcher::createWorker(unsigned int workerId)
{
	return new Worker(*this, workerId);
}

} // namespace isl


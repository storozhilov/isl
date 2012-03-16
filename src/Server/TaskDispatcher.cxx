#include <isl/TaskDispatcher.hxx>
#include <isl/AbstractTask.hxx>
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

TaskDispatcher::TaskDispatcher(AbstractSubsystem * owner, size_t workersCount, size_t maxTaskQueueOverflowSize) :
	AbstractSubsystem(owner),
	_startStopMutex(),
	_workersCount(workersCount),
	_workersCountRwLock(),
	_taskCond(),
	_awaitingWorkersCount(0),
	_maxTaskQueueOverflowSize(maxTaskQueueOverflowSize),
	_maxTaskQueueOverflowSizeRwLock(),
	_tasks(),
	_workers()
{}

TaskDispatcher::~TaskDispatcher()
{
	resetWorkers();
	for (Tasks::iterator i = _tasks.begin(); i != _tasks.end(); ++i) {
		delete (*i);
	}
}

bool TaskDispatcher::perform(AbstractTask * task)
{
	size_t awaitingWorkersCount;
	size_t taskQueueSize;
	size_t currentMaxTaskQueueOverflowSize = maxTaskQueueOverflowSize();
	bool taskPerformed = false;
	{
		MutexLocker locker(_taskCond.mutex());
		awaitingWorkersCount = _awaitingWorkersCount;
		taskQueueSize = _tasks.size();
		if ((awaitingWorkersCount + currentMaxTaskQueueOverflowSize) >= (taskQueueSize + 1)) {
			_tasks.push_front(task);
			_taskCond.wakeOne();
			taskPerformed = true;
		}
	}
	std::wostringstream oss;
	oss << L"Total workers: " << _workers.size() << ", workers awaiting: " << awaitingWorkersCount << L", tasks in pool: " << (taskQueueSize + 1) <<
		L", max task overflow: "  << currentMaxTaskQueueOverflowSize << L", detected tasks overflow: " <<
		((awaitingWorkersCount >= (taskQueueSize + 1)) ? 0 : taskQueueSize + 1 - awaitingWorkersCount);
	if (taskPerformed) {
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
	} else {
		Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
	}
	// TODO The following is thrown "Variant type is not supported..." exception
	/*VariantWFormatter fmt(L"Workers awaiting: $0, tasks in pool: $1, available overflow: $2, tasks overflow: $3");
	fmt.arg(awaitingWorkersCount).arg(taskQueueSize + 1).arg(currentMaxTaskQueueOverflowSize).arg((awaitingWorkersCount >= (taskQueueSize + 1)) ? 0 : taskQueueSize + 1 - awaitingWorkersCount);
	if (taskPerformed) {
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, fmt.compose()));
	} else {
		Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, fmt.compose()));
	}*/
	return taskPerformed;
}

bool TaskDispatcher::perform(const TaskList& taskList)
{
	size_t tasksCount = taskList.size();
	if (tasksCount <= 0) {
		return true;
	}
	size_t awaitingWorkersCount;
	size_t taskQueueSize;
	size_t currentMaxTaskQueueOverflowSize = maxTaskQueueOverflowSize();
	bool tasksPerformed = false;
	{
		MutexLocker locker(_taskCond.mutex());
		awaitingWorkersCount = _awaitingWorkersCount;
		taskQueueSize = _tasks.size();
		if ((awaitingWorkersCount + currentMaxTaskQueueOverflowSize) >= (taskQueueSize + tasksCount)) {
			for (TaskList::const_iterator i = taskList.begin(); i != taskList.end(); ++i) {
				_tasks.push_front(*i);
			}
			_taskCond.wakeAll();
			tasksPerformed = true;
		}
	}
	std::wostringstream oss;
	oss << L"Total workers: " << _workers.size() << ", workers awaiting: " << awaitingWorkersCount << L", tasks to execute: " << tasksCount <<
		L" tasks in pool: " << (taskQueueSize + tasksCount) << L", max task overflow: "  << currentMaxTaskQueueOverflowSize << L", detected tasks overflow: " <<
		((awaitingWorkersCount >= (taskQueueSize + tasksCount)) ? 0 : taskQueueSize + 1 - awaitingWorkersCount);
	if (tasksPerformed) {
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
	} else {
		Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, oss.str()));
	}
	return tasksPerformed;
}

void TaskDispatcher::start()
{
	MutexLocker locker(_startStopMutex);
	setState(IdlingState, StartingState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Starting subsystem"));
	resetWorkers();
	for (size_t i = 0; i < workersCount(); ++i) {
		Worker * newWorker = createWorker(i);
		_workers.push_back(newWorker);
		newWorker->start();
	}
	setState(StartingState, RunningState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been started"));
}

void TaskDispatcher::stop()
{
	MutexLocker locker(_startStopMutex);
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

TaskDispatcher::Worker * TaskDispatcher::createWorker(unsigned int workerId)
{
	return new Worker(*this, workerId);
}

void TaskDispatcher::resetWorkers()
{
	for (Workers::iterator i = _workers.begin(); i != _workers.end(); ++i) {
		delete (*i);
	}
	_workers.clear();
}

/*------------------------------------------------------------------------------
 * TaskDispatcher::Worker
------------------------------------------------------------------------------*/

TaskDispatcher::Worker::Worker(TaskDispatcher& taskDispatcher, unsigned int id) :
	Thread(true),
	_taskDispatcher(taskDispatcher),
	_id(id)
{}
	
void TaskDispatcher::Worker::run()
{
	onStart();
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Worker has been started"));
	while (true) {
		if (shouldTerminate()) {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Task dispatcher termination has been detected before task pick up -> exiting from the worker thread"));
			break;
		}
		std::auto_ptr<AbstractTask> task;
		{
			MutexLocker locker(_taskDispatcher._taskCond.mutex());
			if (_taskDispatcher._tasks.empty()) {
				// Wait for the next task if the task queue is empty
				++_taskDispatcher._awaitingWorkersCount;
				_taskDispatcher._taskCond.wait();
				--_taskDispatcher._awaitingWorkersCount;
				if (!_taskDispatcher._tasks.empty()) {
					// Pick up the next task from the task queue if the task queue is not empty
					task.reset(_taskDispatcher._tasks.back());
					_taskDispatcher._tasks.pop_back();
				}
			} else {
				// Pick up the next task from the task queue if the task queue is not empty
				task.reset(_taskDispatcher._tasks.back());
				_taskDispatcher._tasks.pop_back();
			}
		}
		if (shouldTerminate()) {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Task dispatcher termination has been detected after task pick up -> exiting from the worker thread"));
			break;
		}
		if (task.get()) {
			try {
				task->execute(*this);
			} catch (std::exception& e) {
				Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Task execution error"));
			} catch (...) {
				Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Task execution unknown error"));
			}
		} else {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"No task for worker"));
		}
	}
	onStop();
}

} // namespace isl


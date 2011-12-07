#include <isl/Worker.hxx>
#include <isl/TaskDispatcher.hxx>
#include <isl/Core.hxx>
#include <isl/Exception.hxx>
#include <memory>

namespace isl
{

/*------------------------------------------------------------------------------
 * Worker
------------------------------------------------------------------------------*/

Worker::Worker(TaskDispatcher& taskDispatcher, unsigned int id) :
	Thread(true),
	_taskDispatcher(taskDispatcher),
	_id(id)
{}
	
void Worker::run()
{
	onStart();
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Worker has been started"));
	while (true) {
		if (!keepRunning()) {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Task dispatcher stopping detected before task pick up - exiting from the worker thread"));
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
		if (!keepRunning()) {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Task dispatcher stopping detected after task pick up - exiting from the worker thread"));
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
			Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"No task for worker"));
		}
	}
	onStop();
}

void Worker::onStart()
{}

void Worker::onStop()
{}

bool Worker::keepRunning() const
{
	AbstractSubsystem::State taskDispatcherState = _taskDispatcher.state();
	return (taskDispatcherState == AbstractSubsystem::StartingState) || (taskDispatcherState == AbstractSubsystem::RunningState);
}

} // namespace isl

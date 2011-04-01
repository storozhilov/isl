#include <isl/Worker.hxx>
#include <isl/TaskDispatcher.hxx>
#include <isl/Core.hxx>
#include <isl/Exception.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * Worker
------------------------------------------------------------------------------*/

Worker::Worker(TaskDispatcher& taskDispatcher) :
	Thread(true),
	_taskDispatcher(taskDispatcher)
{}
	
void Worker::run()
{
	onStart();
	while (true) {
		if (!keepRunning()) {
			break;
		}
		std::auto_ptr<AbstractTask> task;
		{
			MutexLocker locker(_taskDispatcher._taskCond.mutex());
			++_taskDispatcher._awaitingWorkersCount;
			_taskDispatcher._taskCond.wait();
			--_taskDispatcher._awaitingWorkersCount;
			if (!_taskDispatcher._tasks.empty()) {
				task.reset(_taskDispatcher._tasks.back());
				_taskDispatcher._tasks.pop_back();
			}
		}
		if (!keepRunning()) {
			break;
		}
		if (task.get()) {
			try {
				task->execute(*this);
			} catch (std::exception& e) {
				Core::errorLog.logException(e, SOURCE_LOCATION_ARGS, L"Task execution error");
			} catch (...) {
				Core::errorLog.logDebug(SOURCE_LOCATION_ARGS, L"Task execution unknown error");
			}
		} else {
			Core::warningLog.logDebug(SOURCE_LOCATION_ARGS, L"No task for worker.");
		}
	}
	onStop();
}

void Worker::onStart()
{}

void Worker::onStop()
{}

bool Worker::keepRunning()
{
	AbstractSubsystem::State taskDispatcherState = _taskDispatcher.state();
	if (!taskDispatcherState.equals<AbstractSubsystem::RunningState>() && !taskDispatcherState.equals<AbstractSubsystem::StartingState>()) {
		if (taskDispatcherState.equals<AbstractSubsystem::StoppingState>()) {
			Core::debugLog.logDebug(SOURCE_LOCATION_ARGS, L"Task dispatcher is stopping - exiting worker thread.");
			return false;
		} else if (taskDispatcherState.equals<AbstractSubsystem::RestartingState>()) {
			Core::debugLog.logDebug(SOURCE_LOCATION_ARGS, L"Task dispatcher is restarting - exiting worker thread.");
			return false;
		} else {
			Core::warningLog.logDebug(SOURCE_LOCATION_ARGS,	L"Task dispatcher is in unexpected \"" +
					taskDispatcherState.value().name() + L"\" state.");
		}
	}
	return true;
}

} // namespace isl

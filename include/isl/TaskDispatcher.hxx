#ifndef ISL__TASK_DISPATCHER__HXX
#define ISL__TASK_DISPATCHER__HXX

#include <isl/common.hxx>
#include <isl/Subsystem.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>
#include <deque>
#include <list>
#include <exception>
#include <sstream>
#include <memory>

namespace isl
{

//! Task dispatcher for executing tasks in the pool of the worker threads
/*!
  This is an <a href="http://en.wikipedia.org/wiki/Active_object">Active object pattern</a> templated extensible implementation.
  Thread creation operation is quite expensive one, so it's reasonable to pre-create pool of worker threads which are
  waiting on condition variable for incoming tasks to execute.
*/
template <typename Task> class BasicTaskDispatcher : public Subsystem
{
public:
	typedef std::list<Task *> TaskList;

	//! WorkerThread thread class
	class WorkerThread : public AbstractThread
	{
	public:
		WorkerThread(BasicTaskDispatcher& taskDispatcher, unsigned int id) :
			AbstractThread(taskDispatcher, true, false /* No auto-stop workers - see BasicTaskDispatcher::beforeStop() event handler */),
			_taskDispatcher(taskDispatcher),
			_id(id)
		{}
		virtual ~WorkerThread()
		{}

		inline BasicTaskDispatcher& taskDispatcher()
		{
			return _taskDispatcher;
		}
		inline unsigned int id() const
		{
			return _id;
		}
	protected:
		//! On start event handler
		virtual void onStart()
		{}
		//! On stop event handler
		virtual void onStop()
		{}
	private:
		WorkerThread();
		WorkerThread(const WorkerThread&);							// No copy

		WorkerThread& operator=(const WorkerThread&);						// No copy

		virtual void run()
		{
			onStart();
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "WorkerThread has been started"));
			while (true) {
				if (shouldTerminate()) {
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Worker thread termination has been detected before task pick up -> exiting from the worker thread"));
					break;
				}
				std::auto_ptr<Task> taskAutoPtr;
				{
					MutexLocker locker(_taskDispatcher._taskCond.mutex());
					if (_taskDispatcher._taskQueue.empty()) {
						// Wait for the next task if the task queue is empty
						++_taskDispatcher._awaitingWorkersCount;
						_taskDispatcher._taskCond.wait();
						--_taskDispatcher._awaitingWorkersCount;
						if (!_taskDispatcher._taskQueue.empty()) {
							// Pick up the next task from the task queue if the task queue is not empty
							taskAutoPtr.reset(_taskDispatcher._taskQueue.back());
							_taskDispatcher._taskQueue.pop_back();
						}
					} else {
						// Pick up the next task from the task queue if the task queue is not empty
						taskAutoPtr.reset(_taskDispatcher._taskQueue.back());
						_taskDispatcher._taskQueue.pop_back();
					}
				}
				if (shouldTerminate()) {
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Worker thread termination has been detected after task pick up -> exiting from the worker thread"));
					break;
				}
				if (taskAutoPtr.get()) {
					try {
						taskAutoPtr->execute(*this);
					} catch (std::exception& e) {
						errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Task execution error"));
					} catch (...) {
						errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Task execution unknown error"));
					}
				} else {
					// TODO Remove it?
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "No task for worker"));
				}
			}
			onStop();
		}

		BasicTaskDispatcher& _taskDispatcher;
		unsigned int _id;
	};
	
	//! Constructs new task dispatcher
	/*!
	  \param owner Pointer to the owner subsystem
	  \param workersAmount WorkerThread threads amount
	  \param maxTaskQueueOverflowSize Max tasks queue overflow
	*/
	BasicTaskDispatcher(Subsystem * owner, size_t workersAmount, size_t maxTaskQueueOverflowSize = 0) :
		Subsystem(owner),
		_workersAmount(workersAmount),
		_maxTaskQueueOverflowSize(maxTaskQueueOverflowSize),
		_taskCond(),
		_awaitingWorkersCount(0),
		_taskQueue(),
		_workers()
	{}
	~BasicTaskDispatcher()
	{
		resetWorkers();
		for (typename TaskQueue::iterator i = _taskQueue.begin(); i != _taskQueue.end(); ++i) {
			delete (*i);
		}
	}
	
	//! Returns workers amount
	inline size_t workersAmount() const
	{
		ReadLocker locker(runtimeParamsRWLock);
		return _workersAmount;
	}
	//! Sets new workers count.
	/*!
	  Subsystem's restart needed to completely apply the new value;

	  \param newValue New workers amount
	*/
	inline void setWorkersAmount(size_t newValue)
	{
		WriteLocker locker(runtimeParamsRWLock);
		_workersAmount = newValue;
	}
	//! Thread-safely returns maximum task queue overflow size.
	inline size_t maxTaskQueueOverflowSize() const
	{
		MutexLocker locker(_taskCond.mutex());
		return _maxTaskQueueOverflowSize;
	}
	//! Thread-safely sets the new maximum task queue overflow size.
	/*!
	  Changes will take place on the next task performing operation

	  \param newValue New maximum task queue overflow size
	*/
	inline void setMaxTaskQueueOverflowSize(size_t newValue)
	{
		MutexLocker locker(_taskCond.mutex());
		_maxTaskQueueOverflowSize = newValue;
	}
	//! Performs a task
	/*!
	  \return True if the task has been successfully passed to workers
	*/
	bool perform(Task * task)
	{
		size_t awaitingWorkersCount;
		size_t taskQueueSize;
		size_t currentMaxTaskQueueOverflowSize;
		bool taskPerformed = false;
		{
			MutexLocker locker(_taskCond.mutex());
			awaitingWorkersCount = _awaitingWorkersCount;
			taskQueueSize = _taskQueue.size();
			if ((awaitingWorkersCount + _maxTaskQueueOverflowSize) >= (taskQueueSize + 1)) {
				_taskQueue.push_front(task);
				_taskCond.wakeOne();
				taskPerformed = true;
			}
			currentMaxTaskQueueOverflowSize = _maxTaskQueueOverflowSize;
		}
		std::ostringstream oss;
		oss << "Total workers: " << _workers.size() << ", workers awaiting: " << awaitingWorkersCount << ", tasks in pool: " << (taskQueueSize + 1) <<
			", max task overflow: "  << currentMaxTaskQueueOverflowSize << ", detected tasks overflow: " <<
			((awaitingWorkersCount >= (taskQueueSize + 1)) ? 0 : taskQueueSize + 1 - awaitingWorkersCount);
		if (taskPerformed) {
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
		} else {
			warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
		}
		return taskPerformed;
	}
	//! Performs a tasks
	/*!
	  \return True if the tasks have been successfully passed to workers
	*/
	bool perform(const TaskList& taskList)
	{
		size_t tasksCount = taskList.size();
		if (tasksCount <= 0) {
			return true;
		}
		size_t awaitingWorkersCount;
		size_t taskQueueSize;
		size_t currentMaxTaskQueueOverflowSize;
		bool tasksPerformed = false;
		{
			MutexLocker locker(_taskCond.mutex());
			awaitingWorkersCount = _awaitingWorkersCount;
			taskQueueSize = _taskQueue.size();
			if ((awaitingWorkersCount + _maxTaskQueueOverflowSize) >= (taskQueueSize + tasksCount)) {
				for (typename TaskList::const_iterator i = taskList.begin(); i != taskList.end(); ++i) {
					_taskQueue.push_front(*i);
				}
				_taskCond.wakeAll();
				tasksPerformed = true;
			}
			currentMaxTaskQueueOverflowSize = _maxTaskQueueOverflowSize;
		}
		std::ostringstream oss;
		oss << "Total workers: " << _workers.size() << ", workers awaiting: " << awaitingWorkersCount << ", tasks to execute: " << tasksCount <<
			" tasks in pool: " << (taskQueueSize + tasksCount) << ", max task overflow: "  << currentMaxTaskQueueOverflowSize << ", detected tasks overflow: " <<
			((awaitingWorkersCount >= (taskQueueSize + tasksCount)) ? 0 : taskQueueSize + 1 - awaitingWorkersCount);
		if (tasksPerformed) {
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
		} else {
			warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, oss.str()));
		}
		return tasksPerformed;
	}
protected:
	//! Creating new worker factory method
	/*!
	  \param workerId Id for the new worker
	  \return Pointer to new worker thread
	*/
	virtual WorkerThread * createWorker(unsigned int workerId)
	{
		return new WorkerThread(*this, workerId);
	}
private:
	BasicTaskDispatcher();
	BasicTaskDispatcher(const BasicTaskDispatcher&);						// No copy

	BasicTaskDispatcher& operator=(const BasicTaskDispatcher&);					// No copy

	typedef std::deque<Task *> TaskQueue;
	typedef std::list<WorkerThread *> WorkerList;

	void resetWorkers()
	{
		for (typename WorkerList::iterator i = _workers.begin(); i != _workers.end(); ++i) {
			delete (*i);
		}
		_workers.clear();
	}

	virtual void beforeStart()
	{
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Starting task dispatcher"));
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Creating workers"));
		for (size_t i = 0; i < workersAmount(); ++i) {
			std::auto_ptr<WorkerThread> newWorkerAutoPtr(createWorker(i));
			_workers.push_back(newWorkerAutoPtr.get());
			newWorkerAutoPtr.release();
		}
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Workers have been created"));
	}
	virtual void afterStart()
	{
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Task dispatcher has been started"));
	}
	virtual void beforeStop()
	{
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Stopping task dispatcher"));
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Waking up workers"));
		{
			for (typename WorkerList::iterator i = _workers.begin(); i != _workers.end(); ++i) {
				(*i)->setShouldTerminate(true);
			}
			{
				MutexLocker locker(_taskCond.mutex());
				_taskCond.wakeAll();
			}
			for (typename WorkerList::iterator i = _workers.begin(); i != _workers.end(); ++i) {
				(*i)->join();
			}
		}
	}
	virtual void afterStop()
	{
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Deleting workers"));
		resetWorkers();
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Workers have been deleted"));
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Task dispatcher has been stopped"));
	}

	size_t _workersAmount;
	size_t _maxTaskQueueOverflowSize;
	mutable WaitCondition _taskCond;
	size_t _awaitingWorkersCount;
	TaskQueue _taskQueue;
	WorkerList _workers;
};

//! Base class for task which worker should execute
class AbstractTask
{
public:
	typedef BasicTaskDispatcher<AbstractTask> TaskDispatcherType;

	AbstractTask()
	{}
	virtual ~AbstractTask()
	{}
	
	virtual void execute(TaskDispatcherType::WorkerThread& worker) = 0;
private:
	AbstractTask(const AbstractTask&);						// No copy

	AbstractTask& operator=(const AbstractTask&);					// No copy
};

typedef BasicTaskDispatcher<AbstractTask> TaskDispatcher;

} // namespace isl

#endif

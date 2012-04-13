#ifndef ISL__TASK_DISPATCHER__HXX
#define ISL__TASK_DISPATCHER__HXX

#include <isl/AbstractSubsystem.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Thread.hxx>
#include <deque>
#include <list>
#include <exception>
#include <sstream>
#include <memory>

namespace isl
{

//! Task dispatcher for executing tasks in the pool of the worker threads
template <typename Task> class BasicTaskDispatcher : public AbstractSubsystem
{
public:
	typedef std::list<Task *> TaskList;

	//! WorkerThread thread class
	class WorkerThread : public SubsystemThread
	{
	public:
		WorkerThread(BasicTaskDispatcher& taskDispatcher, unsigned int id) :
			SubsystemThread(taskDispatcher, true),
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
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"WorkerThread has been started"));
			while (true) {
				if (shouldTerminate()) {
					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Task dispatcher termination has been detected before task pick up -> exiting from the worker thread"));
					break;
				}
				std::auto_ptr<Task> task;
				{
					MutexLocker locker(_taskDispatcher._taskCond.mutex());
					if (_taskDispatcher._taskQueue.empty()) {
						// Wait for the next task if the task queue is empty
						++_taskDispatcher._awaitingWorkersCount;
						_taskDispatcher._taskCond.wait();
						--_taskDispatcher._awaitingWorkersCount;
						if (!_taskDispatcher._taskQueue.empty()) {
							// Pick up the next task from the task queue if the task queue is not empty
							task.reset(_taskDispatcher._taskQueue.back());
							_taskDispatcher._taskQueue.pop_back();
						}
					} else {
						// Pick up the next task from the task queue if the task queue is not empty
						task.reset(_taskDispatcher._taskQueue.back());
						_taskDispatcher._taskQueue.pop_back();
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

		BasicTaskDispatcher& _taskDispatcher;
		unsigned int _id;
	};
	
	//! Constructs new task dispatcher
	/*!
	  \param owner Pointer to the owner subsystem
	  \param workersAmount WorkerThread threads amount
	  \param maxTaskQueueOverflowSize Max tasks queue overflow
	*/
	BasicTaskDispatcher(AbstractSubsystem * owner, size_t workersAmount, size_t maxTaskQueueOverflowSize = 0) :
		AbstractSubsystem(owner),
		_workersAmount(workersAmount),
		_workersCountRwLock(),
		_taskCond(),
		_awaitingWorkersCount(0),
		_maxTaskQueueOverflowSize(maxTaskQueueOverflowSize),
		_maxTaskQueueOverflowSizeRwLock(),
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
	
	//! Thread-safely returns workers amount
	inline size_t workersAmount() const
	{
		ReadLocker locker(_workersCountRwLock);
		return _workersAmount;
	}
	//! Thread-safely sets the new workers count.
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param newValue New workers amount
	*/
	inline void setWorkersAmount(size_t newValue)
	{
		WriteLocker locker(_workersCountRwLock);
		_workersAmount = newValue;
	}
	//! Thread-safely returns maximum task queue overflow size.
	inline size_t maxTaskQueueOverflowSize() const
	{
		ReadLocker locker(_maxTaskQueueOverflowSizeRwLock);
		return _maxTaskQueueOverflowSize;
	}
	//! Thread-safely sets the new maximum task queue overflow size.
	/*!
	  Changes will take place on the next task performing operation
	  \param newValue New maximum task queue overflow size
	*/
	inline void setMaxTaskQueueOverflowSize(size_t newValue)
	{
		WriteLocker locker(_maxTaskQueueOverflowSizeRwLock);
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
		size_t currentMaxTaskQueueOverflowSize = maxTaskQueueOverflowSize();
		bool taskPerformed = false;
		{
			MutexLocker locker(_taskCond.mutex());
			awaitingWorkersCount = _awaitingWorkersCount;
			taskQueueSize = _taskQueue.size();
			if ((awaitingWorkersCount + currentMaxTaskQueueOverflowSize) >= (taskQueueSize + 1)) {
				_taskQueue.push_front(task);
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
		size_t currentMaxTaskQueueOverflowSize = maxTaskQueueOverflowSize();
		bool tasksPerformed = false;
		{
			MutexLocker locker(_taskCond.mutex());
			awaitingWorkersCount = _awaitingWorkersCount;
			taskQueueSize = _taskQueue.size();
			if ((awaitingWorkersCount + currentMaxTaskQueueOverflowSize) >= (taskQueueSize + tasksCount)) {
				for (typename TaskList::const_iterator i = taskList.begin(); i != taskList.end(); ++i) {
					_taskQueue.push_front(*i);
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
protected:
	//! Creating new worker factory method
	/*!
	  \param workerId Id for the new worker
	  \return std::auto_ptr with new worker's pointer
	*/
	virtual std::auto_ptr<WorkerThread> createWorker(unsigned int workerId)
	{
		return std::auto_ptr<WorkerThread>(new WorkerThread(*this, workerId));
	}
private:
	BasicTaskDispatcher();
	BasicTaskDispatcher(const BasicTaskDispatcher&);							// No copy

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
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Starting task dispatcher"));
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Creating workers"));
		for (size_t i = 0; i < workersAmount(); ++i) {
			std::auto_ptr<WorkerThread> newWorkerAutoPtr(createWorker(i));
			_workers.push_back(newWorkerAutoPtr.get());
			newWorkerAutoPtr.release();
		}
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Workers have been created"));
	}
	virtual void afterStart()
	{
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Task dispatcher has been started"));
	}
	virtual void beforeStop()
	{
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Stopping task dispatcher"));
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Waking up workers"));
		{
			MutexLocker locker(_taskCond.mutex());
			_taskCond.wakeAll();
		}
	}
	virtual void afterStop()
	{
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Deleting workers"));
		resetWorkers();
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Workers have been deleted"));
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Task dispatcher has been stopped"));
	}

	size_t _workersAmount;
	mutable ReadWriteLock _workersCountRwLock;
	WaitCondition _taskCond;
	size_t _awaitingWorkersCount;
	size_t _maxTaskQueueOverflowSize;
	mutable ReadWriteLock _maxTaskQueueOverflowSizeRwLock;
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

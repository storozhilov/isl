#ifndef ISL__TASK_DISPATCHER__HXX
#define ISL__TASK_DISPATCHER__HXX

#include <isl/AbstractSubsystem.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Thread.hxx>
#include <deque>
#include <list>

namespace isl
{

class AbstractTask;

//! Task dispatcher for executing tasks in the pool of the worker threads
class TaskDispatcher : public AbstractSubsystem
{
public:
	typedef std::list<AbstractTask *> TaskList;
	//! Constructs new task dispatcher
	/*!
	  \param owner Pointer to the owner subsystem
	  \param workersCount Worker threads count
	  \param maxTaskQueueOverflowSize Max tasks queue overflow
	*/
	TaskDispatcher(AbstractSubsystem * owner, size_t workersCount, size_t maxTaskQueueOverflowSize = 0);
	~TaskDispatcher();
	
	//! Worker thread class
	class Worker : public Thread
	{
	public:
		Worker(TaskDispatcher& taskDispatcher, unsigned int id);

		inline const TaskDispatcher& taskDispatcher() const
		{
			return _taskDispatcher;
		}
		inline unsigned int id() const
		{
			return _id;
		}
	private:
		Worker();
		Worker(const Worker&);								// No copy

		Worker& operator=(const Worker&);						// No copy

		inline bool shouldTerminate() const
		{
			AbstractSubsystem::State taskDispatcherState = _taskDispatcher.state();
			return (taskDispatcherState != AbstractSubsystem::StartingState) && (taskDispatcherState != AbstractSubsystem::RunningState);
		}

		virtual void run();
		virtual void onStart()
		{}
		virtual void onStop()
		{}

		TaskDispatcher& _taskDispatcher;
		unsigned int _id;
	};

	//! Thread-safely returns workers count.
	inline size_t workersCount() const
	{
		ReadLocker locker(_workersCountRwLock);
		return _workersCount;
	}
	//! Thread-safely sets the new workers count.
	/*!
	  Subsystem's restart needed to actually apply new value
	  \param newValue New workers count
	*/
	inline void setWorkersCount(size_t newValue)
	{
		WriteLocker locker(_workersCountRwLock);
		_workersCount = newValue;
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
	bool perform(AbstractTask * task);
	//! Performs a tasks
	/*!
	  \return True if the tasks have been successfully passed to workers
	*/
	bool perform(const TaskList& taskList);							// TODO

	virtual void start();
	virtual void stop();
protected:
	//! Creating new worker foactory method
	/*!
	  \param workerId Id for the new worker
	  \return New worker
	*/
	virtual Worker * createWorker(unsigned int workerId);
private:
	TaskDispatcher();
	TaskDispatcher(const TaskDispatcher&);							// No copy

	TaskDispatcher& operator=(const TaskDispatcher&);					// No copy

	void resetWorkers();

	typedef std::deque<AbstractTask *> Tasks;
	typedef std::list<Worker *> Workers;

	size_t _workersCount;
	mutable ReadWriteLock _workersCountRwLock;
	WaitCondition _taskCond;
	size_t _awaitingWorkersCount;
	size_t _maxTaskQueueOverflowSize;
	mutable ReadWriteLock _maxTaskQueueOverflowSizeRwLock;
	Tasks _tasks;
	Workers _workers;

	friend class Worker;
};

} // namespace isl

#endif


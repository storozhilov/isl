#ifndef ISL__TASK_DISPATCHER__HXX
#define ISL__TASK_DISPATCHER__HXX

#include <isl/AbstractSubsystem.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Thread.hxx>
#include <deque>
#include <list>

namespace isl
{

//class AbstractTask;

//! Task dispatcher for executing tasks in the pool of the worker threads
class TaskDispatcher : public AbstractSubsystem
{
public:
	//! Constructs new task dispatcher
	/*!
	  \param owner Pointer to the owner subsystem
	  \param workersAmount Worker threads amount
	  \param maxTaskQueueOverflowSize Max tasks queue overflow
	*/
	TaskDispatcher(AbstractSubsystem * owner, size_t workersAmount, size_t maxTaskQueueOverflowSize = 0);
	~TaskDispatcher();
	
	//! Worker thread class
	class Worker : public SubsystemThread
	{
	public:
		Worker(TaskDispatcher& taskDispatcher, unsigned int id);
		virtual ~Worker();

		inline TaskDispatcher& taskDispatcher()
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
		Worker();
		Worker(const Worker&);								// No copy

		Worker& operator=(const Worker&);						// No copy

		virtual void run();

		TaskDispatcher& _taskDispatcher;
		unsigned int _id;
	};
	
	//! Base class for task which worker should execute
	class AbstractTask
	{
	public:
		AbstractTask();
		virtual ~AbstractTask();
		
		//! Executes task
		/*!
		  \param worker Reference to worker which executes the task
		*/
		void execute(TaskDispatcher::Worker& worker);
	protected:
		//! Before execute event handler
		virtual void beforeExecute()
		{}
		//! After execute event handler
		virtual void afterExecute()
		{}
		//! Task execution virtual abstract method to override in subclasses
		/*!
		  \param worker Reference to worker which executes the task
		*/
		virtual void executeImpl(Worker& worker) = 0;
	private:
		AbstractTask(const AbstractTask&);						// No copy

		AbstractTask& operator=(const AbstractTask&);					// No copy

		bool _executed;
	};
	typedef std::list<AbstractTask *> TaskList;

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
	bool perform(AbstractTask * task);
	//! Performs a tasks
	/*!
	  \return True if the tasks have been successfully passed to workers
	*/
	bool perform(const TaskList& taskList);
protected:
	//! Creating new worker factory method
	/*!
	  \param workerId Id for the new worker
	  \return New worker
	*/
	virtual Worker * createWorker(unsigned int workerId)
	{
		return new Worker(*this, workerId);
	}
private:
	TaskDispatcher();
	TaskDispatcher(const TaskDispatcher&);							// No copy

	TaskDispatcher& operator=(const TaskDispatcher&);					// No copy

	typedef std::deque<AbstractTask *> Tasks;
	typedef std::list<Worker *> Workers;

	void resetWorkers();

	virtual void beforeStart();
	virtual void afterStart();
	virtual void beforeStop();
	virtual void afterStop();

	size_t _workersAmount;
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


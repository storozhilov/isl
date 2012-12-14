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

#include <isl/MemFunThread.hxx>

namespace isl
{

//! Executes task object's method in separate thread
/*!
  Use this class if you want your <strong>task object</strong>'s method to be executed in a separate thread,
  which is one of the prestarted threads pool. If your task object has 2 or more such methods to execute take a
  look at the MultiTaskDispatcher class instead.

  \note Task dispatcher will automatically dispose all pending tasks on stop() operation without execution.

  \tparam T Task object class

  \sa MultiTaskDispatcher

  \sa <a href="http://en.wikipedia.org/wiki/Active_object">Active object pattern</a>
*/
template <typename T> class TaskDispatcher : public Subsystem
{
public:
	//! Task object's method type definition
	typedef void (T::*Method)(TaskDispatcher<T>&);
private:
	typedef MemFunThread<TaskDispatcher<T> > WorkerThread;

	class PendingTask
	{
	public:
		PendingTask(TaskDispatcher<T>& dispatcher, T * taskPtr, const Method method) :
			_dispatcher(dispatcher),
			_taskAutoPtr(taskPtr),
			_method(method)
		{}
		inline void execute()
		{
			((_taskAutoPtr.get())->*(_method))(_dispatcher);
		}
	private:
		PendingTask();
		PendingTask(const PendingTask&);							// No copy
		PendingTask& operator=(const PendingTask&);						// No copy

		TaskDispatcher<T>& _dispatcher;
		std::auto_ptr<T> _taskAutoPtr;
		Method _method;
	};
	typedef std::deque<PendingTask *> PendingTasksQueue;
public:
	//! Constructs new task dispatcher
	/*!
	  \param owner Pointer to the owner subsystem
	  \param workersAmount Worker threads amount
	*/
	TaskDispatcher(Subsystem * owner, size_t workersAmount) :
		Subsystem(owner),
		_workersAmount(workersAmount),
		_cond(),
		_shouldTerminate(false),
		_workers(),
		_awaitingWorkersCount(0),
		_pendingTasksQueue()
	{}
	virtual ~TaskDispatcher()
	{
		resetWorkers();
		resetPendingTasksQueue();
	}
	//! Returns workers amount
	inline size_t workersAmount() const
	{
		return _workersAmount;
	}
	//! Sets workers amount
	/*!
	  \param newValue New workers amount

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	inline void setWorkersAmount(size_t newValue)
	{
		_workersAmount = newValue;
	}
	//! Returns if the task dispatcher should be terminated
	/*!
	  Call this method periodically during long-live tasks execution for correct subsystem's termination.

	  \note Thread-safe
	*/
	inline bool shouldTerminate() const
	{
		MutexLocker locker(_cond.mutex());
		return _shouldTerminate;
	}
	//! Accepts task for execution it's single method in separate thread
	/*!
	  \param taskAutoPtr Reference to the auto-pointer to task object, which is automatically released if the task has been successfully accepted.
	  \param method Pointer to method of the task to be executed in the separate thread
	  \return TRUE if the task has been successfully accepted

	  \note Thread-safe
	*/
	inline bool perform(std::auto_ptr<T>& taskAutoPtr, Method method)
	{
		//return perform(taskAutoPtr, MethodsContainer(1, method));
		//return true;
		if (!taskAutoPtr.get()) {
			// TODO Maybe to throw an exception???
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Empty pointer to task to execute"));
			return true;
		}
		bool taskPerformed = false;
		{
			MutexLocker locker(_cond.mutex());
			_pendingTasksQueue.push_front(new PendingTask(*this, taskAutoPtr.get(), method));
			taskPerformed = true;
			_cond.wakeOne();
		}
		if (taskPerformed) {
			taskAutoPtr.release();
		} else {
			warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, "No enough workers available"));
		}
		return taskPerformed;
	}
protected:
	virtual void startImpl()
	{
		// Calling ancestor's method
		Subsystem::startImpl();
		_shouldTerminate = false;
		_awaitingWorkersCount = 0;
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Creating and starting workers"));
		for (size_t i = 0; i < _workersAmount; ++i) {
			std::auto_ptr<WorkerThread> newWorkerAutoPtr(new WorkerThread());
			WorkerThread * newWorkerPtr = newWorkerAutoPtr.get();
			_workers.push_back(newWorkerPtr);
			newWorkerAutoPtr.release();
			newWorkerPtr->start(*this, &TaskDispatcher<T>::work);
		}
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Workers have been created and started"));
	}
	virtual void stopImpl()
	{
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Stopping workers"));
		// Waking up all workers
		{
			MutexLocker locker(_cond.mutex());
			_shouldTerminate = true;
			_cond.wakeAll();
		}
		// Waiting for all workers to terminate
		for (typename WorkersContainer::iterator i = _workers.begin(); i != _workers.end(); ++i) {
			(*i)->join();
		}
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Workers have been stopped"));
		// Disposing workers
		resetWorkers();
		// Disposing pending tasks queue
		resetPendingTasksQueue();
		// Calling ancestor's method
		Subsystem::stopImpl();
	}
private:
	TaskDispatcher();
	TaskDispatcher(const TaskDispatcher&);						// No copy

	TaskDispatcher& operator=(const TaskDispatcher&);					// No copy

	typedef std::list<WorkerThread *> WorkersContainer;

	void resetWorkers()
	{
		for (typename WorkersContainer::iterator i = _workers.begin(); i != _workers.end(); ++i) {
			delete (*i);
		}
		_workers.clear();
	}

	void resetPendingTasksQueue()
	{
		for (typename PendingTasksQueue::iterator i = _pendingTasksQueue.begin(); i != _pendingTasksQueue.end(); ++i) {
			delete (*i);
			warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Pending task has been discarded"));
		}
		_pendingTasksQueue.clear();
	}

	void work()
	{
		while (true) {
			std::auto_ptr<PendingTask> pendingTaskAutoPtr;
			{
				MutexLocker locker(_cond.mutex());
				if (_shouldTerminate) {
					return;
				}
				if (_pendingTasksQueue.empty()) {
					// Waiting for the next task if the pending tasks queue is empty
					++_awaitingWorkersCount;
					_cond.wait();
					--_awaitingWorkersCount;
					if (_shouldTerminate) {
						return;
					}
					if (!_pendingTasksQueue.empty()) {
						pendingTaskAutoPtr.reset(_pendingTasksQueue.back());
						_pendingTasksQueue.pop_back();
					}
				} else {
					pendingTaskAutoPtr.reset(_pendingTasksQueue.back());
					_pendingTasksQueue.pop_back();
				}
			}
			if (pendingTaskAutoPtr.get()) {
				pendingTaskAutoPtr->execute();
			} else {
				warningLog().log(LogMessage(SOURCE_LOCATION_ARGS, "No pending task for worker"));
			}
		}
	}

	size_t _workersAmount;
	mutable WaitCondition _cond;
	bool _shouldTerminate;
	WorkersContainer _workers;
	size_t _awaitingWorkersCount;
	PendingTasksQueue _pendingTasksQueue;

	friend class MemFunThread<TaskDispatcher<T> >;
};

} // namespace isl

#endif

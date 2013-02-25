#ifndef ISL__MULTI_TASK_DISPATCHER__HXX
#define ISL__MULTI_TASK_DISPATCHER__HXX

#include <isl/Log.hxx>
#include <isl/Subsystem.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>
#include <isl/MemFunThread.hxx>
#include <deque>
#include <list>

namespace isl
{

//! Executes task object's method(s) in separate thread(s)
/*!
  Use this class if you have 2 or more methods of the <strong>task object</strong> to be executed in the separate threads.
  If your task object has only one such method you should use TaskDispatcher class instead, cause it does
  not need additional mutex per each task object which is used for correct task object disposal.

  \note Task dispatcher will automatically dispose all pending tasks on stop() operation without execution.

  \tparam T Task object class

  \sa TaskDispatcher

  \sa <a href="http://en.wikipedia.org/wiki/Active_object">Active object pattern</a>
*/
template <typename T> class MultiTaskDispatcher : public Subsystem
{
public:
	//! Task object's method type definition
	typedef void (T::*Method)(MultiTaskDispatcher<T>&);
	//! Task object's methods container type definition
	typedef std::list<Method> MethodsContainer;
private:
	class TaskDisposer
	{
	public:
		TaskDisposer(T * taskPtr) :
			_taskAutoPtr(taskPtr),
			_refsCount(0),
			_refsCountMutex()
		{}
		inline size_t incRef()
		{
			MutexLocker locker(_refsCountMutex);
			return ++_refsCount;
		}
		inline size_t decRef()
		{
			MutexLocker locker(_refsCountMutex);
			return --_refsCount;
		}
		inline T * task() const
		{
			return _taskAutoPtr.get();
		}
	private:
		TaskDisposer();
		TaskDisposer(const TaskDisposer&);							// No copy
		TaskDisposer& operator=(const TaskDisposer&);						// No copy

		std::auto_ptr<T> _taskAutoPtr;
		size_t _refsCount;
		Mutex _refsCountMutex;
	};

	class PendingTask
	{
	public:
		PendingTask(MultiTaskDispatcher<T>& dispatcher, TaskDisposer * taskDisposerPtr, const Method method) :
			_dispatcher(dispatcher),
			_taskDisposerPtr(taskDisposerPtr),
			_method(method)
		{
			_taskDisposerPtr->incRef();
		}
		~PendingTask()
		{
			if (_taskDisposerPtr->decRef() <= 0) {
				delete _taskDisposerPtr;
			}
		}
		inline void execute()
		{
			((_taskDisposerPtr->task())->*(_method))(_dispatcher);
		}
	private:
		PendingTask();
		PendingTask(const PendingTask&);							// No copy
		PendingTask& operator=(const PendingTask&);						// No copy

		MultiTaskDispatcher<T>& _dispatcher;
		TaskDisposer * _taskDisposerPtr;
		Method _method;
	};
	typedef std::deque<PendingTask *> PendingTasksQueue;
public:
	//! Constructs new task dispatcher
	/*!
	  \param owner Pointer to the owner subsystem
	  \param workersAmount Worker threads amount
	*/
	MultiTaskDispatcher(Subsystem * owner, size_t workersAmount, const Timeout& clockTimeout = Timeout::defaultTimeout()) :
		Subsystem(owner, clockTimeout),
		_workersAmount(workersAmount),
		_cond(),
		_shouldTerminate(false),
		_workers(),
		_awaitingWorkersCount(0),
		_pendingTasksQueue()
	{}
	virtual ~MultiTaskDispatcher()
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
	//! Awaits for task dispatcher termination
	/*!
	  \param limit Limit timestamp to wait until the termination
	  \returns TRUE if the task dispatcher should be terminated
	*/
	bool awaitTermination(const Timestamp& limit)
	{
		MutexLocker locker(_cond.mutex());
		do {
			if (_shouldTerminate) {
				return true;
			}
		} while (_cond.wait(limit));
		return false;
	}
	//! Accepts task for execution it's method(s) in separate thread(s)
	/*!
	  \param taskAutoPtr Reference to the auto-pointer to task object, which is automatically released if the task has been successfully accepted.
	  \param methods Reference to method(s) of the task to be executed in the separate thread(s)
	  \return TRUE if the task has been successfully accepted

	  \note Thread-safe
	*/
	bool perform(std::auto_ptr<T>& taskAutoPtr, const MethodsContainer& methods)
	{
		if (!taskAutoPtr.get()) {
			// TODO Maybe to throw an exception???
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Empty pointer to task to execute"));
			return true;
		}
		if (methods.empty()) {
			// TODO Maybe to throw an exception???
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "No task methods to execute"));
			return true;
		}
		bool taskPerformed = false;
		{
			MutexLocker locker(_cond.mutex());
			if (_pendingTasksQueue.size() + methods.size() <= _awaitingWorkersCount) {
				std::auto_ptr<TaskDisposer> taskDisposerAutoPtr(new TaskDisposer(taskAutoPtr.get()));
				for (typename MethodsContainer::const_iterator i = methods.begin(); i != methods.end(); ++i) {
					_pendingTasksQueue.push_front(new PendingTask(*this, taskDisposerAutoPtr.get(), (*i)));
				}
				taskDisposerAutoPtr.release();
				taskPerformed = true;
				_cond.wakeAll();
			}
		}
		if (taskPerformed) {
			taskAutoPtr.release();
		} else {
			Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "No enough workers available"));
		}
		return taskPerformed;
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
		return perform(taskAutoPtr, MethodsContainer(1, method));
	}
	//! Accepts task for execution it's two methods in separate threads
	/*!
	  \param taskAutoPtr Reference to the auto-pointer to task object, which is automatically released if the task has been successfully accepted.
	  \param method1 Pointer to the first method of the task to be executed in the separate thread
	  \param method2 Pointer to the second method of the task to be executed in the separate thread
	  \return TRUE if the task has been successfully accepted

	  \note Thread-safe
	*/
	inline bool perform(std::auto_ptr<T>& taskAutoPtr, Method method1, Method method2)
	{
		MethodsContainer c(1, method1);
		c.push_back(method2);
		return perform(taskAutoPtr, c);
	}
	//! Accepts task for execution it's three methods in separate threads
	/*!
	  \param taskAutoPtr Reference to the auto-pointer to task object, which is automatically released if the task has been successfully accepted.
	  \param method1 Pointer to the first method of the task to be executed in the separate thread
	  \param method2 Pointer to the second method of the task to be executed in the separate thread
	  \param method3 Pointer to the third method of the task to be executed in the separate thread
	  \return TRUE if the task has been successfully accepted

	  \note Thread-safe
	*/
	inline bool perform(std::auto_ptr<T>& taskAutoPtr, Method method1, Method method2, Method method3)
	{
		MethodsContainer c(1, method1);
		c.push_back(method2);
		c.push_back(method3);
		return perform(taskAutoPtr, c);
	}
	//! Accepts task for execution it's four methods in separate threads
	/*!
	  \param taskAutoPtr Reference to the auto-pointer to task object, which is automatically released if the task has been successfully accepted.
	  \param method1 Pointer to the first method of the task to be executed in the separate thread
	  \param method2 Pointer to the second method of the task to be executed in the separate thread
	  \param method3 Pointer to the third method of the task to be executed in the separate thread
	  \param method4 Pointer to the forth method of the task to be executed in the separate thread
	  \return TRUE if the task has been successfully accepted

	  \note Thread-safe
	*/
	inline bool perform(std::auto_ptr<T>& taskAutoPtr, Method method1, Method method2, Method method3, Method method4)
	{
		MethodsContainer c(1, method1);
		c.push_back(method2);
		c.push_back(method3);
		c.push_back(method4);
		return perform(taskAutoPtr, c);
	}
	//! Starts subsystem
	virtual void start()
	{
		// Calling ancestor's method
		Subsystem::start();
		_shouldTerminate = false;
		_awaitingWorkersCount = 0;
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Creating and starting workers"));
		for (size_t i = 0; i < _workersAmount; ++i) {
			std::auto_ptr<MemFunThread> newWorkerAutoPtr(new MemFunThread());
			MemFunThread * newWorkerPtr = newWorkerAutoPtr.get();
			_workers.push_back(newWorkerPtr);
			newWorkerAutoPtr.release();
			newWorkerPtr->start(*this, &MultiTaskDispatcher<T>::work);
		}
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Workers have been created and started"));
	}
	//! Stops subsystem
	virtual void stop()
	{
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Stopping workers"));
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
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Workers have been stopped"));
		// Disposing workers
		resetWorkers();
		// Disposing pending tasks queue
		resetPendingTasksQueue();
		// Calling ancestor's method
		Subsystem::stop();
	}
private:
	MultiTaskDispatcher();
	MultiTaskDispatcher(const MultiTaskDispatcher&);						// No copy

	MultiTaskDispatcher& operator=(const MultiTaskDispatcher&);					// No copy

	typedef std::list<MemFunThread *> WorkersContainer;

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
			Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Pending task has been discarded"));
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
			}
		}
	}

	size_t _workersAmount;
	mutable WaitCondition _cond;
	bool _shouldTerminate;
	WorkersContainer _workers;
	size_t _awaitingWorkersCount;
	PendingTasksQueue _pendingTasksQueue;

	friend class MemFunThread;
};

} // namespace isl

#endif

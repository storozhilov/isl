#ifndef ISL__THREAD__HXX
#define ISL__THREAD__HXX

#include <isl/ReadWriteLock.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>
#include <new>
#include <pthread.h>
#ifdef __FreeBSD__
#include <pthread_np.h>
#endif
#include <errno.h>

namespace isl
{

//! Class for object's member function execution in a separate thread
/*!
  Use this class if you want an object's method to be executed in the separate thread.
  
  \note The behaviour is undefined when the new thread has been started before the completion of the previous one.

  \sa AbstractThread, FunctorThread
*/
class Thread
{
private:
	template <typename T> class Functor
	{
	public:
		Functor(Thread& thread, T& obj, void (T::*method)()) :
			_thread(thread),
			_obj(&obj),
			_method0(method),
			_method1(0)
		{}
		Functor(Thread& thread, T& obj, void (T::*method)(Thread&)) :
			_thread(thread),
			_obj(&obj),
			_method0(0),
			_method1(method)
		{}

		void operator()()
		{
			if (_method0) {
				((_obj)->*(_method0))();
			} else if (_method1) {
				((_obj)->*(_method1))(_thread);
			} else {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "No pointer to member function to execute in separate thread"));
			}
		}
	private:
		Functor();

		Thread& _thread;
		T * _obj;
		void (T::*_method0)();
		void (T::*_method1)(Thread&);
	};
public:
	//! Thread's handle type
	typedef pthread_t Handle;

	//! Constructs a thread
	/*!
	  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
	  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
	*/
	Thread(bool isTrackable = false, bool awaitStartup = false) :
		_function(0),
		_functor(0),
		_thread(),
		_isTrackable(isTrackable),
		_awaitStartup(awaitStartup),
		_isRunning(false),
		_isRunningRWLockAutoPtr(isTrackable ? new ReadWriteLock() : 0),
		_awaitStartupCondAutoPtr(awaitStartup ? new WaitCondition() : 0)
	{}
	//! Virtual destructor
	virtual ~Thread()
	{
		if (_functor) {
			operator delete(_functor);
		}
	}

	//! Returns thread's opaque handle
	inline Handle handle() const
	{
		return _thread;
	}
	//! Inspects if the thread is trackable
	/*!
	  \note Thread-safe
	*/
	inline bool isTrackable() const
	{
		return _isTrackable;
	}
	//! Inspects if the launching thread will wait until new thread is started
	/*!
	  \note Thread-safe
	*/
	inline bool awaitStartup() const
	{
		return _awaitStartup;
	}
	//! Starts function or functor's <tt>void operator(void)</tt> execution in a new thread
	/*!
	  \param fun Function/functor to execute

	  \tparam T C-function or functor which <tt>void operator()()</tt> is to be executed in a separate thread

	  \note Thread-unsafe
	*/
	template <typename T> void start(T& fun)
	{
		std::auto_ptr<WriteLocker> isRunningLockerAutoPtr;
		if (_isTrackable) {
			isRunningLockerAutoPtr.reset(new WriteLocker(*_isRunningRWLockAutoPtr.get()));
			if (_isRunning) {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "FunctorThread is already running"));
			}
			_isRunning = true;
		}
		_function = reinterpret_cast<void *>(&fun);
		if (_awaitStartup) {
			MutexLocker locker(_awaitStartupCondAutoPtr->mutex());
			if (int errorCode = pthread_create(&_thread, NULL, executeFunction<T>, this)) {
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCreate, errorCode));
			}
			_awaitStartupCondAutoPtr->wait();
		} else {
			if (int errorCode = pthread_create(&_thread, NULL, executeFunction<T>, this)) {
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCreate, errorCode));
			}
		}
	}
	//! Start object's member function execution in a new thread
	/*!
	  \param obj Reference to object
	  \param fun Pointer to object's member function to execute

	  \tparam T Class, which member function is to be executed in a separate thread

	  \note Thread-unsafe
	*/
	template <typename T> void start(T& obj, void (T::*fun)())
	{
		std::auto_ptr<WriteLocker> isRunningLockerAutoPtr;
		if (_isTrackable) {
			isRunningLockerAutoPtr.reset(new WriteLocker(*_isRunningRWLockAutoPtr.get()));
			if (_isRunning) {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread is already running"));
			}
			_isRunning = true;
		}
		_functor = operator new(sizeof(Functor<T>));	// Allocating buffer for functor
		new (_functor) Functor<T>(*this, obj, fun);	// Constructing functor in buffer with "placement new" operator
		if (_awaitStartup) {
			MutexLocker locker(_awaitStartupCondAutoPtr->mutex());
			if (int errorCode = pthread_create(&_thread, NULL, executeFunctor<T>, this)) {
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCreate, errorCode));
			}
			_awaitStartupCondAutoPtr->wait();
		} else {
			if (int errorCode = pthread_create(&_thread, NULL, executeFunctor<T>, this)) {
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCreate, errorCode));
			}
		}
	}
	//! Start object's member function execution in a new thread
	/*!
	  \param obj Reference to object
	  \param fun Pointer to object's member function to execute

	  \tparam T Class, which member function is to be executed in a separate thread

	  \note Thread-unsafe
	*/
	template <typename T> void start(T& obj, void (T::*fun)(Thread&))
	{
		std::auto_ptr<WriteLocker> isRunningLockerAutoPtr;
		if (_isTrackable) {
			isRunningLockerAutoPtr.reset(new WriteLocker(*_isRunningRWLockAutoPtr.get()));
			if (_isRunning) {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread is already running"));
			}
			_isRunning = true;
		}
		_functor = operator new(sizeof(Functor<T>));	// Allocating buffer for functor
		new (_functor) Functor<T>(*this, obj, fun);	// Constructing functor in buffer with "placement new" operator
		if (_awaitStartup) {
			MutexLocker locker(_awaitStartupCondAutoPtr->mutex());
			if (int errorCode = pthread_create(&_thread, NULL, executeFunctor<T>, this)) {
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCreate, errorCode));
			}
			_awaitStartupCondAutoPtr->wait();
		} else {
			if (int errorCode = pthread_create(&_thread, NULL, executeFunctor<T>, this)) {
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCreate, errorCode));
			}
		}
	}
	//! Join a thread and wait for it's termination
	/*!
	  \note Thread-unsafe
	*/
	void join()
	{
		if (pthread_equal(_thread, pthread_self())) {
			return;
		}
		if (int errorCode = pthread_join(_thread, NULL)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadJoin, errorCode));
		}
	}
	//! Join a thread and wait for it's termination until timeout expired
	/*!
	  \param limit Limit timestamp to wait until thread's termination
	  \return TRUE if the thread to join has been finished it's execution until limit timestamp

	  \note Thread-unsafe
	*/
	bool join(const Timestamp& limit)
	{
		if (pthread_equal(_thread, pthread_self())) {
			return true;
		}
		int errorCode = pthread_timedjoin_np(_thread, NULL, &limit.timeSpec());
		switch (errorCode) {
			case 0:
				return true;
			case ETIMEDOUT:
				return false;
			default:
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadTimedJoinNp, errorCode));
		}
	}
	//! Join a thread and wait for it's termination until timeout expired
	/*!
	  \param timeout Timeout to wait for thread's termination
	  \param timeoutLeft Memory location where time interval which is remains after thread's termination or 0 otherwise
	  \return TRUE if the thread to join has been finished it's execution during timeout

	  \note Thread-unsafe
	*/
	bool join(const Timeout& timeout, Timeout * timeoutLeft = 0)
	{
		Timestamp limit = Timestamp::limit(timeout);
		bool result = join(limit);
		if (timeoutLeft) {
			*timeoutLeft = result ? limit.leftTo() : Timeout();
		}
		return result;
	}
	//! Inspects if the thread is in running state
	/*!
	  \note Thread-safe
	*/
	bool isRunning() const
	{
		if (!_isTrackable) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread is not trackable"));
		}
		ReadLocker locker(*_isRunningRWLockAutoPtr.get());
		return _isRunning;
	}

	//! Returns a thread handle of the current thread
	static Handle self()
	{
		return pthread_self();
	}
private:
	template <typename T> static void * executeFunction(void * arg)
	{
		Thread * threadPtr = reinterpret_cast<Thread *>(arg);
		if (threadPtr->_awaitStartup) {
			MutexLocker locker(threadPtr->_awaitStartupCondAutoPtr->mutex());
			threadPtr->_awaitStartupCondAutoPtr->wakeOne();
		}
		bool isTrackable = threadPtr->_isTrackable;
		T * functorPtr = reinterpret_cast<T *>(threadPtr->_function);
		(*(functorPtr))();
		threadPtr->_function = 0;
		if (isTrackable) {
			WriteLocker locker(*threadPtr->_isRunningRWLockAutoPtr.get());
			threadPtr->_isRunning = false;
		}
		return 0;
	}
	template <typename T> static void * executeFunctor(void * arg)
	{
		Thread * threadPtr = reinterpret_cast<Thread *>(arg);
		if (threadPtr->_awaitStartup) {
			MutexLocker locker(threadPtr->_awaitStartupCondAutoPtr->mutex());
			threadPtr->_awaitStartupCondAutoPtr->wakeOne();
		}
		bool isTrackable = threadPtr->_isTrackable;
		Functor<T> * f = reinterpret_cast<Functor<T> *>(threadPtr->_functor);
		(*f)();					// Executing functor
		f->~Functor<T>();			// Explicit functor's destructor call
		operator delete(threadPtr->_functor);	// Deallocating functor's buffer
		threadPtr->_functor = 0;
		if (isTrackable) {
			WriteLocker locker(*threadPtr->_isRunningRWLockAutoPtr.get());
			threadPtr->_isRunning = false;
		}
		return 0;
	}

	void * _function;
	void * _functor;
	pthread_t _thread;
	const bool _isTrackable;
	const bool _awaitStartup;
	bool _isRunning;
	mutable std::auto_ptr<ReadWriteLock> _isRunningRWLockAutoPtr;
	std::auto_ptr<WaitCondition> _awaitStartupCondAutoPtr;
};

} // namespace isl

#endif

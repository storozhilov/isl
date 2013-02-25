#ifndef ISL__MEM_FUN_THREAD__HXX
#define ISL__MEM_FUN_THREAD__HXX

#include <isl/AbstractThread.hxx>
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
class MemFunThread
{
private:
	template <typename T> class Functor
	{
	public:
		Functor(MemFunThread& thread, T& obj, void (T::*fun)()) :
			_thread(thread),
			_obj(&obj),
			_fun0(fun),
			_fun1(0)
		{}
		Functor(MemFunThread& thread, T& obj, void (T::*fun)(MemFunThread&)) :
			_thread(thread),
			_obj(&obj),
			_fun0(0),
			_fun1(fun)
		{}

		void operator()()
		{
			if (_fun0) {
				((_obj)->*(_fun0))();
			} else if (_fun1) {
				((_obj)->*(_fun1))(_thread);
			} else {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "No pointer to member function to execute in separate thread"));
			}
		}
	private:
		Functor();

		MemFunThread& _thread;
		T * _obj;
		void (T::*_fun0)();
		void (T::*_fun1)(MemFunThread&);
	};
public:
	//! Constructs a thread
	/*!
	  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
	  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
	*/
	MemFunThread(bool isTrackable = false, bool awaitStartup = false) :
		_functor(0),
		_thread(),
		_isTrackable(isTrackable),
		_awaitStartup(awaitStartup),
		_isRunning(false),
		_isRunningRWLockAutoPtr(isTrackable ? new ReadWriteLock() : 0),
		_awaitStartupCondAutoPtr(awaitStartup ? new WaitCondition() : 0)
	{}
	//! Virtual destructor
	virtual ~MemFunThread()
	{
		if (_functor) {
			operator delete(_functor);
		}
	}

	//! Returns thread's opaque handle
	inline pthread_t handle() const
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
		if (_functor) {
			operator delete(_functor);
		}
		_functor = operator new(sizeof(Functor<T>));
		new (_functor) Functor<T>(*this, obj, fun);
		if (_awaitStartup) {
			MutexLocker locker(_awaitStartupCondAutoPtr->mutex());
			if (int errorCode = pthread_create(&_thread, NULL, execute<T>, this)) {
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCreate, errorCode));
			}
			_awaitStartupCondAutoPtr->wait();
		} else {
			if (int errorCode = pthread_create(&_thread, NULL, execute<T>, this)) {
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
	template <typename T> void start(T& obj, void (T::*fun)(MemFunThread&))
	{
		std::auto_ptr<WriteLocker> isRunningLockerAutoPtr;
		if (_isTrackable) {
			isRunningLockerAutoPtr.reset(new WriteLocker(*_isRunningRWLockAutoPtr.get()));
			if (_isRunning) {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread is already running"));
			}
			_isRunning = true;
		}
		if (_functor) {
			operator delete(_functor);
		}
		_functor = operator new(sizeof(Functor<T>));
		new (_functor) Functor<T>(*this, obj, fun);
		if (_awaitStartup) {
			MutexLocker locker(_awaitStartupCondAutoPtr->mutex());
			if (int errorCode = pthread_create(&_thread, NULL, execute<T>, this)) {
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCreate, errorCode));
			}
			_awaitStartupCondAutoPtr->wait();
		} else {
			if (int errorCode = pthread_create(&_thread, NULL, execute<T>, this)) {
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
private:
	template <typename T> static void * execute(void * arg)
	{
		MemFunThread * threadPtr = reinterpret_cast<MemFunThread *>(arg);
		if (threadPtr->_awaitStartup) {
			MutexLocker locker(threadPtr->_awaitStartupCondAutoPtr->mutex());
			threadPtr->_awaitStartupCondAutoPtr->wakeOne();
		}
		bool isTrackable = threadPtr->_isTrackable;
		Functor<T> * f = reinterpret_cast<Functor<T> *>(threadPtr->_functor);
		(*f)();
		if (isTrackable) {
			WriteLocker locker(*threadPtr->_isRunningRWLockAutoPtr.get());
			threadPtr->_isRunning = false;
		}
		return 0;
	}

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

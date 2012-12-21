#ifndef ISL__MEM_FUN_THREAD__HXX
#define ISL__MEM_FUN_THREAD__HXX

#include <isl/common.hxx>
#include <isl/AbstractThread.hxx>
#include <isl/ReadWriteLock.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>
#include <pthread.h>
#include <errno.h>

namespace isl
{

//! Templated class for object's member function execution in a separate thread
/*!
  Use this class if you want an object's method to be executed in the separate thread.
  
  \note The behaviour is undefined when the new thread has been started before the completion of the previous one.

  \tparam T Class, which member function is to be executed in a separate thread

  \sa AbstractThread, FunctorThread
*/
template <typename T> class MemFunThread
{
public:
	//! Pointer to member function type definition (no argumants)
	typedef void (T::*MemFunPtr0)();
	//! Pointer to member function type definition (one argument)
	typedef void (T::*MemFunPtr1)(MemFunThread<T>& thread);

	//! Constructs a thread
	/*!
	  \param catchException If TRUE then to catch exceptions occured during function/functor execution (TODO Remove it?)
	  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
	  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
	*/
	MemFunThread(bool catchException = false, bool isTrackable = false, bool awaitStartup = false) :
		_obj(),
		_fun0(),
		_fun1(),
		_thread(),
		_catchException(catchException),
		_isTrackable(isTrackable),
		_awaitStartup(awaitStartup),
		_isRunning(false),
		_isRunningRWLockAutoPtr(isTrackable ? new ReadWriteLock() : 0),
		_awaitStartupCondAutoPtr(awaitStartup ? new WaitCondition() : 0)
	{}
	//! Virtual destructor
	virtual ~MemFunThread()
	{}

	//! Returns thread's opaque handle
	inline pthread_t handle() const
	{
		return _thread;
	}
	//! Inspects if the thread is catching exceptions occured during function/functor execution
	/*!
	  \note Thread-safe
	*/
	inline bool catchException() const
	{
		return _catchException;
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

	  \note Thread-unsafe
	*/
	void start(T& obj, MemFunPtr0 fun)
	{
		std::auto_ptr<WriteLocker> isRunningLockerAutoPtr;
		if (_isTrackable) {
			isRunningLockerAutoPtr.reset(new WriteLocker(*_isRunningRWLockAutoPtr.get()));
			if (_isRunning) {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread is already running"));
			}
			_isRunning = true;
		}
		_obj = &obj;
		_fun0 = fun;
		_fun1 = 0;
		if (_awaitStartup) {
			MutexLocker locker(_awaitStartupCondAutoPtr->mutex());
			if (int errorCode = pthread_create(&_thread, NULL, execute, this)) {
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCreate, errorCode));
			}
			_awaitStartupCondAutoPtr->wait();
		} else {
			if (int errorCode = pthread_create(&_thread, NULL, execute, this)) {
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCreate, errorCode));
			}
		}
	}
	//! Start object's member function execution in a new thread
	/*!
	  \param obj Reference to object
	  \param fun Pointer to object's member function to execute

	  \note Thread-unsafe
	*/
	void start(T& obj, MemFunPtr1 fun)
	{
		std::auto_ptr<WriteLocker> isRunningLockerAutoPtr;
		if (_isTrackable) {
			isRunningLockerAutoPtr.reset(new WriteLocker(*_isRunningRWLockAutoPtr.get()));
			if (_isRunning) {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread is already running"));
			}
			_isRunning = true;
		}
		_obj = &obj;
		_fun0 = 0;
		_fun1 = fun;
		if (_awaitStartup) {
			MutexLocker locker(_awaitStartupCondAutoPtr->mutex());
			if (int errorCode = pthread_create(&_thread, NULL, execute, this)) {
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCreate, errorCode));
			}
			_awaitStartupCondAutoPtr->wait();
		} else {
			if (int errorCode = pthread_create(&_thread, NULL, execute, this)) {
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
	  \param timeout Timeout to wait for thread's termination
	  \return TRUE if the thread to join has been finished it's execution during timeout

	  \note Thread-unsafe
	*/
	bool join(const Timeout& timeout)
	{
		if (pthread_equal(_thread, pthread_self())) {
			return true;
		}
		timespec timeoutLimit = timeout.limit();
		int errorCode = pthread_timedjoin_np(_thread, NULL, &timeoutLimit);
		switch (errorCode) {
			case 0:
				return true;
			case ETIMEDOUT:
				return false;
			default:
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadTimedJoinNp, errorCode));
		}
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
	static void * execute(void * arg)
	{
		//MemFunThreadType * threadPtr = reinterpret_cast<MemFunThreadType *>(arg);
		MemFunThread<T> * threadPtr = reinterpret_cast<MemFunThread<T> *>(arg);
		if (threadPtr->_awaitStartup) {
			MutexLocker locker(threadPtr->_awaitStartupCondAutoPtr->mutex());
			threadPtr->_awaitStartupCondAutoPtr->wakeOne();
		}
		bool isTrackable = threadPtr->_isTrackable;
		if (threadPtr->_catchException) {
			try {
				if (threadPtr->_fun0) {
					((threadPtr->_obj)->*(threadPtr->_fun0))();
				} else {
					((threadPtr->_obj)->*(threadPtr->_fun1))(*threadPtr);
				}
			} catch (std::exception& e) {
				errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Thread execution exception occured"));
			} catch (...) {
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Thread execution unknown exception occured"));
			}
		} else {
			if (threadPtr->_fun0) {
				((threadPtr->_obj)->*(threadPtr->_fun0))();
			} else {
				((threadPtr->_obj)->*(threadPtr->_fun1))(*threadPtr);
			}
		}
		if (isTrackable) {
			WriteLocker locker(*threadPtr->_isRunningRWLockAutoPtr.get());
			threadPtr->_isRunning = false;
		}
		return 0;
	}

	T * _obj;
	MemFunPtr0 _fun0;
	MemFunPtr1 _fun1;
	pthread_t _thread;
	const bool _catchException;
	const bool _isTrackable;
	const bool _awaitStartup;
	bool _isRunning;
	mutable std::auto_ptr<ReadWriteLock> _isRunningRWLockAutoPtr;
	std::auto_ptr<WaitCondition> _awaitStartupCondAutoPtr;
};

} // namespace isl

#endif

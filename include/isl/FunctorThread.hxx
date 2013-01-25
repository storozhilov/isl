#ifndef ISL__FUNCTOR_THREAD__HXX
#define ISL__FUNCTOR_THREAD__HXX

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
#ifdef __FreeBSD__
#include <pthread_np.h>
#endif
#include <errno.h>

namespace isl
{

//! Templated class for function/functor execution in a separate thread
/*!
  Use this class if you want a regular C-finction or Functor's object to be executed in the separate thread.

  \note The behaviour is undefined when the new thread has been started before the completion of the previous one.

  \sa AbstractThread, MemFunThread
*/
class FunctorThread
{
public:
	//! Constructs a thread
	/*!
	  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
	  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
	*/
	FunctorThread(bool isTrackable = false, bool awaitStartup = false) :
		_f(0),
		_thread(),
		_isTrackable(isTrackable),
		_awaitStartup(awaitStartup),
		_isRunning(false),
		_isRunningRWLockAutoPtr(isTrackable ? new ReadWriteLock() : 0),
		_awaitStartupCondAutoPtr(awaitStartup ? new WaitCondition() : 0)
	{}
	//! Virtual destructor
	virtual ~FunctorThread()
	{}

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
	//! Starts function or functor's <tt>void operator(void)</tt> execution in a new thread
	/*!
	  \param f Function/functor to execute

	  \tparam F C-function or functor which <tt>void operator()()</tt> is to be executed in a separate thread

	  \note Thread-unsafe
	*/
	template <typename F> void start(F& f)
	{
		std::auto_ptr<WriteLocker> isRunningLockerAutoPtr;
		if (_isTrackable) {
			isRunningLockerAutoPtr.reset(new WriteLocker(*_isRunningRWLockAutoPtr.get()));
			if (_isRunning) {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "FunctorThread is already running"));
			}
			_isRunning = true;
		}
		_f = reinterpret_cast<void *>(&f);
		if (_awaitStartup) {
			MutexLocker locker(_awaitStartupCondAutoPtr->mutex());
			if (int errorCode = pthread_create(&_thread, NULL, execute<F>, this)) {
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCreate, errorCode));
			}
			_awaitStartupCondAutoPtr->wait();
		} else {
			if (int errorCode = pthread_create(&_thread, NULL, execute<F>, this)) {
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
			throw Exception(Error(SOURCE_LOCATION_ARGS, "FunctorThread is not trackable"));
		}
		ReadLocker locker(*_isRunningRWLockAutoPtr.get());
		return _isRunning;
	}
private:
	template <typename F> static void * execute(void * arg)
	{
		FunctorThread * threadPtr = reinterpret_cast<FunctorThread *>(arg);
		if (threadPtr->_awaitStartup) {
			MutexLocker locker(threadPtr->_awaitStartupCondAutoPtr->mutex());
			threadPtr->_awaitStartupCondAutoPtr->wakeOne();
		}
		bool isTrackable = threadPtr->_isTrackable;
		F * functorPtr = reinterpret_cast<F *>(threadPtr->_f);
		(*(functorPtr))();
		if (isTrackable) {
			WriteLocker locker(*threadPtr->_isRunningRWLockAutoPtr.get());
			threadPtr->_isRunning = false;
		}
		return 0;
	}

	void * _f;
	pthread_t _thread;
	const bool _isTrackable;
	const bool _awaitStartup;
	bool _isRunning;
	mutable std::auto_ptr<ReadWriteLock> _isRunningRWLockAutoPtr;
	std::auto_ptr<WaitCondition> _awaitStartupCondAutoPtr;
};

} // namespace isl

#endif

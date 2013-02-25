#include <isl/AbstractThread.hxx>
#include <isl/ReadWriteLock.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>
#include <errno.h>
#ifdef __FreeBSD__
#include <pthread_np.h>
#endif

namespace isl
{

AbstractThread::AbstractThread(bool isTrackable, bool awaitStartup) :
	_thread(),
	_isTrackable(isTrackable),
	_awaitStartup(awaitStartup),
	_isRunning(false),
	_isRunningRWLockAutoPtr(isTrackable ? new ReadWriteLock() : 0),
	_awaitStartupCondAutoPtr(awaitStartup ? new WaitCondition() : 0)
{}

AbstractThread::~AbstractThread()
{}

void AbstractThread::start()
{
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

void AbstractThread::join()
{
	if (pthread_equal(_thread, pthread_self())) {
		return;
	}
	if (int errorCode = pthread_join(_thread, NULL)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadJoin, errorCode));
	}
}

bool AbstractThread::join(const Timestamp& limit)
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

bool AbstractThread::join(const Timeout& timeout, Timeout * timeoutLeft)
{
	Timestamp limit = Timestamp::limit(timeout);
	bool result = join(limit);
	if (timeoutLeft) {
		*timeoutLeft = result ? limit.leftTo() : Timeout();
	}
	return result;
}

bool AbstractThread::isRunning() const
{
	if (!_isTrackable) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread is not trackable"));
	}
	ReadLocker locker(*_isRunningRWLockAutoPtr.get());
	return _isRunning;
}

void * AbstractThread::execute(void * arg)
{
	AbstractThread * threadPtr = reinterpret_cast<AbstractThread *>(arg);
	if (threadPtr->_awaitStartup) {
		MutexLocker locker(threadPtr->_awaitStartupCondAutoPtr->mutex());
		threadPtr->_awaitStartupCondAutoPtr->wakeOne();
	}
	if (threadPtr->_isTrackable) {
		WriteLocker locker(*threadPtr->_isRunningRWLockAutoPtr.get());
		if (threadPtr->_isRunning) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread is already running"));
		}
		threadPtr->_isRunning = true;
	}
	threadPtr->run();
	if (threadPtr->_isTrackable) {
		WriteLocker locker(*threadPtr->_isRunningRWLockAutoPtr.get());
		threadPtr->_isRunning = false;
	}
	return 0;
}

} // namespace

#include <isl/common.hxx>
#include <isl/AbstractThread.hxx>
#include <isl/ReadWriteLock.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/LogMessage.hxx>
#include <isl/ExceptionLogMessage.hxx>
#include <errno.h>

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

bool AbstractThread::join(const Timeout& timeout)
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
	try {
		threadPtr->run();
	} catch (std::exception& e) {
		errorLog().log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, "Thread execution error"));
	} catch (...) {
		errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Thread execution unknown error"));
	}
	if (threadPtr->_isTrackable) {
		WriteLocker locker(*threadPtr->_isRunningRWLockAutoPtr.get());
		threadPtr->_isRunning = false;
	}
	return 0;
}

} // namespace

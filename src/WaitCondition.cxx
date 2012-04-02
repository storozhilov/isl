#include <isl/WaitCondition.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/Time.hxx>
#include <iostream>
#include <errno.h>

namespace isl
{

/*------------------------------------------------------------------------------
 * WaitCondition
------------------------------------------------------------------------------*/

WaitCondition::WaitCondition() :
	_cond(),
	_mutex()
{
	if (int errorCode = pthread_cond_init(&_cond, NULL)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCondInit, errorCode));
	}
}

WaitCondition::~WaitCondition()
{
	if (int errorCode = pthread_cond_destroy(&_cond)) {
		std::wcerr << SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCondDestroy, errorCode).message() << std::endl;
	}
}

Mutex& WaitCondition::mutex()
{
	return _mutex;
}

void WaitCondition::wait()
{
	if (int errorCode = pthread_cond_wait(&_cond, &(_mutex._mutex))) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCondWait, errorCode));
	}
}

bool WaitCondition::wait(const Timeout& timeout)
{
	if (timeout.isZero()) {
		return false;
	}
	timespec timeoutLimit = timeout.limit();
	int errorCode = pthread_cond_timedwait(&_cond, &(_mutex._mutex), &timeoutLimit);
	switch (errorCode) {
		case 0:
			return true;
		case ETIMEDOUT:
			return false;
		default:
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCondTimedWait, errorCode));
	}
}

void WaitCondition::wakeOne()
{
	if (int errorCode = pthread_cond_signal(&_cond)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCondSignal, errorCode));
	}
}

void WaitCondition::wakeAll()
{
	if (int errorCode = pthread_cond_broadcast(&_cond)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCondBroadcast, errorCode));
	}
}

} // namespace isl


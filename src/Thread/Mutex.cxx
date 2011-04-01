#include <isl/Mutex.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <errno.h>
#include <iostream>

namespace isl
{

/*------------------------------------------------------------------------------
 * Mutex
------------------------------------------------------------------------------*/

Mutex::Mutex()
{
	if (int errorCode = pthread_mutex_init(&_mutex, NULL)) {
		throw Exception(SystemCallError(SystemCallError::PThreadMutexInit, errorCode, SOURCE_LOCATION_ARGS));
	}
}

Mutex::~Mutex()
{
	if (int errorCode = pthread_mutex_destroy(&_mutex)) {
		std::wcerr << SystemCallError(SystemCallError::PThreadMutexDestroy, errorCode, SOURCE_LOCATION_ARGS).message() << std::endl;
	}
}

void Mutex::lock()
{
	if (int errorCode = pthread_mutex_lock(&_mutex)) {
		throw Exception(SystemCallError(SystemCallError::PThreadMutexLock, errorCode, SOURCE_LOCATION_ARGS));
	}
}

bool Mutex::tryLock()
{
	int errorCode = pthread_mutex_trylock(&_mutex);
	switch (errorCode) {
		case 0:
			return true;
		case EBUSY:
			return false;
		default:
			throw Exception(SystemCallError(SystemCallError::PThreadMutexTryLock, errorCode, SOURCE_LOCATION_ARGS));
	}
}

bool Mutex::tryLock(const Timeout& timeout)
{
	if ((timeout.seconds() == 0) && (timeout.nanoSeconds() == 0)) {
		return tryLock();
	}
	timespec timeoutLimit = timeout.limit();
	int errorCode = pthread_mutex_timedlock(&_mutex, &timeoutLimit);
	switch (errorCode) {
		case 0:
			return true;
		case ETIMEDOUT:
			return false;
		default:
			throw Exception(SystemCallError(SystemCallError::PThreadMutexTimedLock, errorCode, SOURCE_LOCATION_ARGS));
	}
}

void Mutex::unlock()
{
	if (int errorCode = pthread_mutex_unlock(&_mutex)) {
		throw Exception(SystemCallError(SystemCallError::PThreadMutexUnlock, errorCode, SOURCE_LOCATION_ARGS));
	}
}

/*------------------------------------------------------------------------------
 * MutexLocker
------------------------------------------------------------------------------*/

MutexLocker::MutexLocker(Mutex& mutex) :
	_mutex(mutex)
{
	_mutex.lock();
}

MutexLocker::~MutexLocker()
{
	_mutex.unlock();
}

/*------------------------------------------------------------------------------
 * MutexUnlocker
------------------------------------------------------------------------------*/

MutexUnlocker::MutexUnlocker(Mutex& mutex) :
	_mutex(mutex)
{}

MutexUnlocker::~MutexUnlocker()
{
	_mutex.unlock();
}

} // namespace isl


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
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadMutexInit, errorCode));
	}
}

Mutex::~Mutex()
{
	if (int errorCode = pthread_mutex_destroy(&_mutex)) {
		std::cerr << SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadMutexDestroy, errorCode).message() << std::endl;
	}
}

void Mutex::lock()
{
	if (int errorCode = pthread_mutex_lock(&_mutex)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadMutexLock, errorCode));
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
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadMutexTryLock, errorCode));
	}
}

bool Mutex::tryLock(const Timeout& timeout)
{
	if (timeout.isZero()) {
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
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadMutexTimedLock, errorCode));
	}
}

void Mutex::unlock()
{
	if (int errorCode = pthread_mutex_unlock(&_mutex)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadMutexUnlock, errorCode));
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


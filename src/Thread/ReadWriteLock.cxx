#include <isl/ReadWriteLock.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/Time.hxx>
#include <errno.h>
#include <iostream>

namespace isl
{

/*------------------------------------------------------------------------------
 * ReadWriteLock class
------------------------------------------------------------------------------*/

ReadWriteLock::ReadWriteLock()
{
	if (int errorCode = pthread_rwlock_init(&_lock, NULL)) {
		throw Exception(SystemCallError(SystemCallError::PThreadRWLockInit, errorCode, SOURCE_LOCATION_ARGS));
	}
}

ReadWriteLock::~ReadWriteLock()
{
	if (int errorCode = pthread_rwlock_destroy(&_lock)) {
		std::wcerr << SystemCallError(SystemCallError::PThreadRWLockDestroy, errorCode, SOURCE_LOCATION_ARGS).message() << std::endl;
	}
}

void ReadWriteLock::lockForRead()
{
	if (int errorCode = pthread_rwlock_rdlock(&_lock)) {
		throw Exception(SystemCallError(SystemCallError::PThreadRWLockRdLock, errorCode, SOURCE_LOCATION_ARGS));
	}
}

void ReadWriteLock::lockForWrite()
{
	if (int errorCode = pthread_rwlock_wrlock(&_lock)) {
		throw Exception(SystemCallError(SystemCallError::PThreadRWLockWrLock, errorCode, SOURCE_LOCATION_ARGS));
	}
}

bool ReadWriteLock::tryLockForRead()
{
	int errorCode = pthread_rwlock_tryrdlock(&_lock);
	switch (errorCode) {
		case 0:
			return true;
		case EBUSY:
			return false;
		default:
			throw Exception(SystemCallError(SystemCallError::PThreadRWLockTryRdLock, errorCode, SOURCE_LOCATION_ARGS));
	}
}

bool ReadWriteLock::tryLockForRead(const Timeout& timeout)
{
	if ((timeout.seconds() == 0) && (timeout.nanoSeconds() == 0)) {
		return tryLockForRead();
	}
	timespec timeoutLimit = timeout.limit();
	int errorCode = pthread_rwlock_timedrdlock(&_lock, &timeoutLimit);
	switch (errorCode) {
		case 0:
			return true;
		case ETIMEDOUT:
			return false;
		default:
			throw Exception(SystemCallError(SystemCallError::PThreadRWLockTimedRdLock, errorCode, SOURCE_LOCATION_ARGS));
	}
}

bool ReadWriteLock::tryLockForWrite()
{
	int errorCode = pthread_rwlock_trywrlock(&_lock);
	switch (errorCode) {
		case 0:
			return true;
		case EBUSY:
			return false;
		default:
			throw Exception(SystemCallError(SystemCallError::PThreadRWLockTryWrLock, errorCode, SOURCE_LOCATION_ARGS));
	}
}

bool ReadWriteLock::tryLockForWrite(const Timeout& timeout)
{
	if ((timeout.seconds() == 0) && (timeout.nanoSeconds() == 0)) {
		return tryLockForRead();
	}
	timespec timeoutLimit = timeout.limit();
	int errorCode = pthread_rwlock_timedwrlock(&_lock, &timeoutLimit);
	switch (errorCode) {
		case 0:
			return true;
		case ETIMEDOUT:
			return false;
		default:
			throw Exception(SystemCallError(SystemCallError::PThreadRWLockTimedWrLock, errorCode, SOURCE_LOCATION_ARGS));
	}
}

void ReadWriteLock::unlock()
{
	if (int errorCode = pthread_rwlock_unlock(&_lock)) {
		throw Exception(SystemCallError(SystemCallError::PThreadRWLockUnlock, errorCode, SOURCE_LOCATION_ARGS));
	}
}

/*------------------------------------------------------------------------------
 * ReadLocker class
------------------------------------------------------------------------------*/

ReadLocker::ReadLocker(ReadWriteLock& lock) :
	_lock(lock)
{
	_lock.lockForRead();
}

ReadLocker::~ReadLocker()
{
	_lock.unlock();
}

/*------------------------------------------------------------------------------
 * WriteLocker class
------------------------------------------------------------------------------*/

WriteLocker::WriteLocker(ReadWriteLock& lock) :
	_lock(lock)
{
	_lock.lockForWrite();
}

WriteLocker::~WriteLocker()
{
	_lock.unlock();
}

/*------------------------------------------------------------------------------
 * ReadWriteUnlocker class
------------------------------------------------------------------------------*/

ReadWriteUnlocker::ReadWriteUnlocker(ReadWriteLock& lock) :
	_lock(lock)
{}

ReadWriteUnlocker::~ReadWriteUnlocker()
{
	_lock.unlock();
}

} // namespace


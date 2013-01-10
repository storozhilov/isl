#include <isl/ReadWriteLock.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
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
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadRWLockInit, errorCode));
	}
}

ReadWriteLock::~ReadWriteLock()
{
	if (int errorCode = pthread_rwlock_destroy(&_lock)) {
		std::cerr << SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadRWLockDestroy, errorCode).message() << std::endl;
	}
}

void ReadWriteLock::lockForRead()
{
	if (int errorCode = pthread_rwlock_rdlock(&_lock)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadRWLockRdLock, errorCode));
	}
}

void ReadWriteLock::lockForWrite()
{
	if (int errorCode = pthread_rwlock_wrlock(&_lock)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadRWLockWrLock, errorCode));
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
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadRWLockTryRdLock, errorCode));
	}
}

bool ReadWriteLock::tryLockForRead(const Timestamp& limit)
{
	int errorCode = pthread_rwlock_timedrdlock(&_lock, &limit.timeSpec());
	switch (errorCode) {
		case 0:
			return true;
		case ETIMEDOUT:
			return false;
		default:
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadRWLockTimedRdLock, errorCode));
	}
}

bool ReadWriteLock::tryLockForRead(const Timeout& timeout, Timeout * timeoutLeft)
{
	if (timeout.isZero()) {
		if (timeoutLeft) {
			*timeoutLeft = Timeout();
		}
		return tryLockForRead();
	}
	Timestamp limit = Timestamp::limit(timeout);
	bool result = tryLockForRead(limit);
	if (timeoutLeft) {
		*timeoutLeft = result ? limit.leftTo() : Timeout();
	}
	return result;
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
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadRWLockTryWrLock, errorCode));
	}
}

bool ReadWriteLock::tryLockForWrite(const Timestamp& limit)
{
	int errorCode = pthread_rwlock_timedwrlock(&_lock, &limit.timeSpec());
	switch (errorCode) {
		case 0:
			return true;
		case ETIMEDOUT:
			return false;
		default:
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadRWLockTimedWrLock, errorCode));
	}
}

bool ReadWriteLock::tryLockForWrite(const Timeout& timeout, Timeout * timeoutLeft)
{
	if (timeout.isZero()) {
		if (timeoutLeft) {
			*timeoutLeft = Timeout();
		}
		return tryLockForWrite();
	}
	Timestamp limit = Timestamp::limit(timeout);
	bool result = tryLockForWrite(limit);
	if (timeoutLeft) {
		*timeoutLeft = result ? limit.leftTo() : Timeout();
	}
	return result;
}

void ReadWriteLock::unlock()
{
	if (int errorCode = pthread_rwlock_unlock(&_lock)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadRWLockUnlock, errorCode));
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


#ifndef ISL__READ_WRITE_LOCK__HXX
#define ISL__READ_WRITE_LOCK__HXX

#include <isl/Timeout.hxx>
#include <pthread.h>

#include <set>

namespace isl
{

/*------------------------------------------------------------------------------
 * ReadWriteLock
------------------------------------------------------------------------------*/

class ReadWriteLock
{
public:
	ReadWriteLock();
	~ReadWriteLock();

	void lockForRead();
	void lockForWrite();
	bool tryLockForRead();
	bool tryLockForRead(const Timeout& timeout);
	bool tryLockForWrite();
	bool tryLockForWrite(const Timeout& timeout);
	void unlock();
private:
	ReadWriteLock(const ReadWriteLock&);					// No copy

	void operator=(const ReadWriteLock&);					// No copy

	pthread_rwlock_t _lock;
};

/*------------------------------------------------------------------------------
 * ReadLocker
------------------------------------------------------------------------------*/

class ReadLocker
{
public:
	ReadLocker(ReadWriteLock& lock);
	~ReadLocker();
private:
	ReadLocker();
	ReadLocker(const ReadLocker&);						// No copy

	void operator=(const ReadLocker&);					// No copy

	ReadWriteLock& _lock;
};

/*------------------------------------------------------------------------------
 * WriteLocker
------------------------------------------------------------------------------*/

class WriteLocker
{
public:
	WriteLocker(ReadWriteLock& _lock);
	~WriteLocker();
private:
	WriteLocker();
	WriteLocker(const WriteLocker&);					// No copy

	void operator=(const WriteLocker&);					// No copy

	ReadWriteLock& _lock;
};

/*------------------------------------------------------------------------------
 * ReadWriteUnlocker
------------------------------------------------------------------------------*/

class ReadWriteUnlocker
{
public:
	ReadWriteUnlocker(ReadWriteLock& _lock);
	~ReadWriteUnlocker();
private:
	ReadWriteUnlocker();
	ReadWriteUnlocker(const ReadWriteUnlocker&);					// No copy

	void operator=(const ReadWriteUnlocker&);					// No copy

	ReadWriteLock& _lock;
};

} // namespace isl

#endif


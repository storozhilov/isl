#ifndef ISL__READ_WRITE_LOCK__HXX
#define ISL__READ_WRITE_LOCK__HXX

#include <isl/Timeout.hxx>
#include <pthread.h>

#include <set>

namespace isl
{

//! Read-write lock interthread synchronization object
class ReadWriteLock
{
public:
	ReadWriteLock();
	~ReadWriteLock();
	//! Locks R/W-lock for read
	void lockForRead();
	//! Locks R/W-lock fro write
	void lockForWrite();
	//! Tries to lock R/W-lock for read
	/*!
	  \return TRUE if the lock has been successfully obtained
	*/
	bool tryLockForRead();
	//! Tries to lock R/W-lock for read during the supplied timeout
	/*!
	  \param timeout Timeout to wait for lock to be available
	  \return TRUE if the lock has been successfully obtained ot FALSE if the timeout has been expired
	*/
	bool tryLockForRead(const Timeout& timeout);
	//! Tries to lock R/W-lock for write
	/*!
	  \return TRUE if the lock has been successfully obtained
	*/
	bool tryLockForWrite();
	//! Tries to lock R/W-lock for write during the supplied timeout
	/*!
	  \param timeout Timeout to wait for lock to be available
	  \return TRUE if the lock has been successfully obtained ot FALSE if the timeout has been expired
	*/
	bool tryLockForWrite(const Timeout& timeout);
	//! Unlocks R/W-lock
	void unlock();
private:
	ReadWriteLock(const ReadWriteLock&);					// No copy

	void operator=(const ReadWriteLock&);					// No copy

	pthread_rwlock_t _lock;
};

//! Locking R/W-lock for read helper class
/*!
  Locks R/W-lock in constructor and unlocks it in the dectructor
*/
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

//! Locking R/W-lock for write helper class
/*!
  Locks R/W-lock in constructor and unlocks it in the dectructor
*/
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

//! Unlocking R/W-lock for write helper class
/*!
  Unlocks R/W-lock in the dectructor
*/
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

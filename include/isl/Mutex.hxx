#ifndef ISL__MUTEX__HXX
#define ISL__MUTEX__HXX

#include <isl/Timeout.hxx>
#include <pthread.h>

namespace isl
{

//! Mutex inter-thread synchronization object
class Mutex
{
public:
	Mutex();
	~Mutex();
	//! Locks the mutex
	void lock();
	//! Tries to lock the mutex
	/*!
	  \return TRUE if the mutex has been successfully locked or FALSE otherwise
	*/
	bool tryLock();
	//! Tries to lock the mutex during a timeout
	/*!
	  \param timeout Timeout to wait for mutex locking
	  \return TRUE if the mutex has been successfully locked or FALSE if the timeout has been expired
	*/
	bool tryLock(const Timeout& timeout);
	//! Unlocks the mutex
	void unlock();
private:
	Mutex(const Mutex&);							// No copy

	Mutex& operator=(const Mutex&);						// No copy

	pthread_mutex_t _mutex;

	friend class WaitCondition;
};

//! Locks the mutex in constructor and unlocks one in destructor
class MutexLocker 
{
public:
	//! Creates locker and locks the mutex
	/*!
	  \param mutex Reference to the mutex to lock
	*/
	MutexLocker(Mutex& mutex);
	//! Destroys locker and unlocks the mutex
	~MutexLocker();
private:
	MutexLocker();
	MutexLocker(const MutexLocker&);					// No copy

	MutexLocker& operator=(const MutexLocker&);				// No copy

	Mutex& _mutex;
};

//! Unlocks the mutex in it's descructor
class MutexUnlocker 
{
public:
	//! Creates the unlocker
	/*!
	  \param mutex Reference to the mutex to further unlock in destructor
	*/
	MutexUnlocker(Mutex& mutex);
	//! Destroys locker and unlocks the mutex
	~MutexUnlocker();
private:
	MutexUnlocker();
	MutexUnlocker(const MutexUnlocker&);					// No copy

	MutexUnlocker& operator=(const MutexUnlocker&);				// No copy

	Mutex& _mutex;
};

} // namespace isl

#endif

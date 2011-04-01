#ifndef ISL__MUTEX__HXX
#define ISL__MUTEX__HXX

#include <isl/Timeout.hxx>
#include <pthread.h>

#include <set>

namespace isl
{

/*------------------------------------------------------------------------------
 * Mutex
------------------------------------------------------------------------------*/

class Mutex
{
public:
	Mutex();
	~Mutex();

	void lock();
	bool tryLock();
	bool tryLock(const Timeout& timeout);
	void unlock();
private:
	Mutex(const Mutex&);							// No copy

	Mutex& operator=(const Mutex&);						// No copy

	pthread_mutex_t _mutex;

	friend class WaitCondition;
};

/*------------------------------------------------------------------------------
 * MutexLocker
------------------------------------------------------------------------------*/

class MutexLocker 
{
public:
	MutexLocker(Mutex& mutex);
	~MutexLocker();
private:
	MutexLocker();
	MutexLocker(const MutexLocker&);					// No copy

	MutexLocker& operator=(const MutexLocker&);				// No copy

	Mutex& _mutex;
};

/*------------------------------------------------------------------------------
 * MutexUnlocker
------------------------------------------------------------------------------*/

class MutexUnlocker 
{
public:
	MutexUnlocker(Mutex& mutex);
	~MutexUnlocker();
private:
	MutexUnlocker();
	MutexUnlocker(const MutexUnlocker&);					// No copy

	MutexUnlocker& operator=(const MutexUnlocker&);				// No copy

	Mutex& _mutex;
};

} // namespace isl

#endif


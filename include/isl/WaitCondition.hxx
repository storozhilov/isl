#ifndef ISL__WAIT_CONDITION__HXX
#define ISL__WAIT_CONDITION__HXX

#include "Mutex.hxx"

namespace isl
{

/*------------------------------------------------------------------------------
 * WaitCondition
------------------------------------------------------------------------------*/
//! Condition variable inter-thread synchronization object
class WaitCondition
{
public:
	WaitCondition();
	~WaitCondition();

	//! Returns a reference to the internal mutex to be used with condition variable
	Mutex& mutex();
	//! Waits for condition variable wake-up
	void wait();
	//! Waits for condition variable wake-up until timeout expires
	/*!
	  /param timeout Timeout to wait for wake-up
	  /param timeoutLeft Time interval which is remains after wake up
	*/
	bool wait(const Timeout& timeout, Timeout * timeoutLeft = 0);
	//bool wait(const Timeout& timeout);
	//! Wakes up one thread, which waits on condition variable
	void wakeOne();
	//! Wakes up all threads, which wait on condition variable
	void wakeAll();
private:
	WaitCondition(const WaitCondition&);					// No copy

	WaitCondition& operator=(const WaitCondition&);				// No copy

	pthread_cond_t _cond;
	Mutex _mutex;
};

} // namespace isl

#endif


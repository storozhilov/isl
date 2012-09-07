#ifndef ISL__WAIT_CONDITION__HXX
#define ISL__WAIT_CONDITION__HXX

#include "Mutex.hxx"
#include <memory>

namespace isl
{

//! Condition variable inter-thread synchronization object
class WaitCondition
{
public:
	//! Constructs a condition variable with an internal mutex
	WaitCondition();
	//! Constructs a condition variable with user-provided mutex
	WaitCondition(Mutex& mutex);
	//! Destructor
	~WaitCondition();

	//! Returns a reference to the mutex to be used with condition variable
	inline Mutex& mutex()
	{
		return _providedMutexPtr ? *_providedMutexPtr : *_internalMutexAutoPtr.get();
	}
	//! Waits for condition variable wake-up
	void wait();
	//! Waits for condition variable wake-up until timeout expires
	/*!
	  \param timeout Timeout to wait for wake-up
	  \param timeoutLeft Memory location where time interval which is remains after wake up is to be put or 0 otherwise
	*/
	bool wait(const Timeout& timeout, Timeout * timeoutLeft = 0);
	//! Wakes up one thread, which waits on condition variable
	void wakeOne();
	//! Wakes up all threads, which wait on condition variable
	void wakeAll();
private:
	WaitCondition(const WaitCondition&);					// No copy

	WaitCondition& operator=(const WaitCondition&);				// No copy

	pthread_cond_t _cond;
	Mutex * _providedMutexPtr;
	std::auto_ptr<Mutex> _internalMutexAutoPtr;
};

} // namespace isl

#endif

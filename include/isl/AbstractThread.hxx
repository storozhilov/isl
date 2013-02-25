#ifndef ISL__ABSTRACT_THREAD__HXX
#define ISL__ABSTRACT_THREAD__HXX

#include <isl/Timestamp.hxx>
#include <pthread.h>
#include <memory>

namespace isl
{

class ReadWriteLock;
class WaitCondition;

//! Standalone thread abstract base class
/*!
  Use this class if you want an object, which is running it's virtual method in the separate thread.
  
  \note The behaviour is undefined when the new thread has been started before the completion of the previous one.

  \sa MemFunThread, FunctorThread
*/
class AbstractThread
{
public:
	//! Constructs a thread
	/*!
	  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
	  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
	*/
	AbstractThread(bool isTrackable = false, bool awaitStartup = false);
	//! Destructor
	virtual ~AbstractThread();
	//! Returns thread's opaque handle
	inline pthread_t handle() const
	{
		return _thread;
	}
	//! Inspects if the thread is trackable
	inline bool isTrackable() const
	{
		return _isTrackable;
	}
	//! Inspects if the launching thread will wait until new thread is started
	inline bool awaitStartup() const
	{
		return _awaitStartup;
	}
	//! Start thread execution
	void start();
	//! Join a thread and wait for it's termination
	void join();
	//! Join a thread and wait for it's termination until timeout expired
	/*!
	  \param limit Limit timestamp to wait until thread's termination
	  \return TRUE if the thread to join has been finished it's execution until limit timestamp

	  \note Thread-unsafe
	*/
	bool join(const Timestamp& limit);
	//! Join a thread and wait for it's termination until timeout expired
	/*!
	  \param timeout Timeout to wait for thread's termination
	  \param timeoutLeft Memory location where time interval which is remains after thread's termination or 0 otherwise
	  \return TRUE if the thread to join has been finished it's execution during timeout

	  \note Thread-unsafe
	*/
	bool join(const Timeout& timeout, Timeout * timeoutLeft = 0);
	//! Inspects if the thread is in running state
	bool isRunning() const;
protected:
	//! Thread execution abstract virtual method to override in subclasses
	virtual void run() = 0;
private:
	AbstractThread(const AbstractThread&);						// No copy

	AbstractThread& operator=(const AbstractThread&);				// No copy

	static void * execute(void * arg);

	pthread_t _thread;
	const bool _isTrackable;
	const bool _awaitStartup;
	bool _isRunning;
	mutable std::auto_ptr<ReadWriteLock> _isRunningRWLockAutoPtr;
	std::auto_ptr<WaitCondition> _awaitStartupCondAutoPtr;
};

} // namespace isl

#endif

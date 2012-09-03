#ifndef ISL__ABSTRACT_THREAD__HXX
#define ISL__ABSTRACT_THREAD__HXX

#include <isl/ReadWriteLock.hxx>
#include <pthread.h>

namespace isl
{

class WaitCondition;

//! Thread abstract base class
class AbstractThread
{
public:
	//! Constructs a thread
	AbstractThread();
	//! Constructs a thread with await startup flag
	/*!
	  \param awaitStartup If TRUE, then the launching thread will wait until new thread is started
	*/
	AbstractThread(bool awaitStartup);
	virtual ~AbstractThread();
	//! Start thread execution
	void start();
	//! Join a thread and wait for it's termination
	void join();
	//! Join a thread and wait for it's termination until timeout expired
	/*!
	  \param timeout Timeout to wait for thread's termination
	*/
	bool join(const Timeout& timeout);
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
	bool _isRunning;
	mutable ReadWriteLock _isRunningRWLock;
	bool _awaitStartup;
	WaitCondition * _awaitStartupCond;

};

} // namespace isl

#endif

#ifndef ISL__THREAD__HXX
#define ISL__THREAD__HXX

#include <isl/ReadWriteLock.hxx>
#include <pthread.h>

namespace isl
{

class WaitCondition;

class Thread
{
public:
	Thread();
	Thread(bool awaitStartup);
	virtual ~Thread();

	void start();
	void join();
	bool join(const Timeout& timeout);
	bool isRunning() const;
	void execute();
protected:
	virtual void run() = 0;
private:
	Thread(const Thread&);						// No copy

	Thread& operator=(const Thread&);				// No copy

	pthread_t _thread;
	bool _isRunning;
	mutable ReadWriteLock _isRunningRWLock;
	bool _awaitStartup;
	WaitCondition * _awaitStartupCond;

};

extern "C" void * Thread_execute(void * arg);

} // namespace isl

#endif

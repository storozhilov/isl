#ifndef ISL__WORKER__HXX
#define ISL__WORKER__HXX

#include <isl/Thread.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * Worker
------------------------------------------------------------------------------*/

class TaskDispatcher;

class Worker : public Thread
{
public:
	Worker(TaskDispatcher& taskDispatcher);
protected:
	inline TaskDispatcher& taskDispatcher() const
	{
		return _taskDispatcher;
	}
private:
	Worker();
	Worker(const Worker&);								// No copy

	Worker& operator=(const Worker&);						// No copy

	virtual void run();
	virtual void onStart();
	virtual void onStop();

	bool keepRunning();

	TaskDispatcher& _taskDispatcher;
};

} // namespace isl

#endif



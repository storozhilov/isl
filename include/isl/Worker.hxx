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
	Worker(TaskDispatcher& taskDispatcher, unsigned int id);

	inline const TaskDispatcher& taskDispatcher() const
	{
		return _taskDispatcher;
	}
	inline unsigned int id() const
	{
		return _id;
	}
private:
	Worker();
	Worker(const Worker&);								// No copy

	Worker& operator=(const Worker&);						// No copy

	bool keepRunning() const;

	virtual void run();
	virtual void onStart();
	virtual void onStop();

	TaskDispatcher& _taskDispatcher;
	unsigned int _id;
};

} // namespace isl

#endif



#ifndef ISL__TASK_DISPATCHER__HXX
#define ISL__TASK_DISPATCHER__HXX

#include <isl/AbstractSubsystem.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/AbstractTask.hxx>
#include <isl/Thread.hxx>
#include <isl/Worker.hxx>
#include <deque>
#include <list>

namespace isl
{

class TaskDispatcher : public AbstractSubsystem
{
public:
	typedef std::list<AbstractTask *> TaskList;
	//! Constructs new task dispatcher
	/*!
	  \param owner Pointer to the owner subsystem
	  \param workersCount Worker threads count
	  \param maxTaskQueueOverflowSize Max tasks queue overflow
	*/
	TaskDispatcher(AbstractSubsystem * owner, unsigned int workersCount, unsigned int maxTaskQueueOverflowSize = 0);
	~TaskDispatcher();

	inline unsigned int workersCount() const
	{
		return _workersCount;
	}
	inline unsigned int maxTaskQueueOverflowSize() const
	{
		return _maxTaskQueueOverflowSize;
	}
	bool perform(AbstractTask * task);
	bool perform(const TaskList& taskList);							// TODO

	virtual bool start();
	virtual void stop();
	virtual bool restart();
protected:
	virtual Worker * createWorker(unsigned int workerId);
private:
	TaskDispatcher();
	TaskDispatcher(const TaskDispatcher&);							// No copy

	TaskDispatcher& operator=(const TaskDispatcher&);					// No copy

	typedef std::deque<AbstractTask *> Tasks;
	typedef std::list<Worker *> Workers;

	unsigned int _workersCount;
	WaitCondition _taskCond;
	unsigned int _awaitingWorkersCount;
	unsigned int _maxTaskQueueOverflowSize;
	Tasks _tasks;
	Workers _workers;

	friend class Worker;
};

} // namespace isl

#endif


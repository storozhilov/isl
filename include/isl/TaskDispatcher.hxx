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

/*------------------------------------------------------------------------------
 * TaskDispatcher
------------------------------------------------------------------------------*/

class TaskDispatcher : public AbstractSubsystem
{
public:
	TaskDispatcher(AbstractSubsystem * owner, unsigned int workersCount, unsigned int availableTaskOverflow = 0);
	~TaskDispatcher();

	bool perform(AbstractTask * task);
	unsigned int availableTaskOverflow() const;
	void setAvailableTaskOverflow(unsigned int newValue);
protected:
	virtual Worker * createWorker(unsigned int workerId);
private:
	TaskDispatcher();
	TaskDispatcher(const TaskDispatcher&);							// No copy

	TaskDispatcher& operator=(const TaskDispatcher&);					// No copy

	typedef std::deque<AbstractTask *> Tasks;
	typedef std::list<Worker *> Workers;

	virtual void onStartCommand();
	virtual void onStopCommand();
	//virtual Worker * createWorker(unsigned int workerId);

	unsigned int _workersCount;
	WaitCondition _taskCond;
	unsigned int _awaitingWorkersCount;
	unsigned int _availableTaskOverflow;
	mutable ReadWriteLock _availableTaskOverflowRwLock;
	Tasks _tasks;
	Workers _workers;

	friend class Worker;
};

} // namespace isl

#endif


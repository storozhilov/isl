#ifndef ISL__ABSTRACT_TASK__HXX
#define ISL__ABSTRACT_TASK__HXX

#include <isl/Worker.hxx>

namespace isl
{

class AbstractTask
{
public:
	AbstractTask();
	virtual ~AbstractTask();

	void execute(Worker& worker);
protected:
	virtual void executeImplementation(Worker& worker) = 0;
private:
	AbstractTask(const AbstractTask&);						// No copy

	AbstractTask& operator=(const AbstractTask&);					// No copy

	bool _executed;
};

} // namespace isl

#endif

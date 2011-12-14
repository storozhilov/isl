#include <isl/AbstractTask.hxx>
#include <isl/Exception.hxx>

#include <stdexcept>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractTask
------------------------------------------------------------------------------*/

AbstractTask::AbstractTask() :
	_executed(false)
{}

AbstractTask::~AbstractTask()
{}

void AbstractTask::execute(TaskDispatcher::Worker& worker)
{
	if (_executed) {
		// TODO
		throw std::runtime_error("Task has been already executed");
	}
	try {
		executeImplementation(worker);
	} catch (...) {
		_executed = true;
		throw;
	}
	_executed = true;
}

} // namespace isl


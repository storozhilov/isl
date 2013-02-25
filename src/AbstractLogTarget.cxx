#include <isl/AbstractLogTarget.hxx>
#include <isl/Log.hxx>

namespace isl
{

AbstractLogTarget::AbstractLogTarget(AbstractLogger& logger) :
	_logger(logger),
	_logs()
{
	_logger.registerTarget(*this);
}

AbstractLogTarget::~AbstractLogTarget()
{
	_logger.unregisterTarget(*this);
	LogsContainer logs = _logs;
	for (LogsContainer::iterator i = logs.begin(); i != logs.end(); ++i) {
		(*i)->disconnect(*this);
	}
}

} // namespace isl

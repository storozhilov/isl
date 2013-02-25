#include <isl/Log.hxx>
#include <isl/AbstractLogTarget.hxx>

#ifndef ISL__ERROR_LOG_PREFIX
#define ISL__ERROR_LOG_PREFIX "ERROR"
#endif

#ifndef ISL__WARNING_LOG_PREFIX
#define ISL__WARNING_LOG_PREFIX "WARNING"
#endif

#ifndef ISL__DEBUG_LOG_PREFIX
#define ISL__DEBUG_LOG_PREFIX "DEBUG"
#endif

namespace isl
{

Log::Log() :
	_prefix(),
	_targets()
{}

Log::Log(const std::string& prefix) :
	_prefix(prefix),
	_targets()
{}

Log::~Log()
{}

void Log::connect(AbstractLogTarget& target)
{
	TargetsContainer::iterator pos = _targets.find(&target);
	if (pos != _targets.end()) {
		// TODO
		return;
	}
	_targets.insert(&target);
	target._logs.insert(this);
}

void Log::disconnect(AbstractLogTarget& target)
{
	TargetsContainer::iterator pos = _targets.find(&target);
	if (pos == _targets.end()) {
		// TODO
		return;
	}
	_targets.erase(&target);
	target._logs.erase(this);
}

void Log::log(const AbstractLogMessage& msg)
{
	for (TargetsContainer::iterator i = _targets.begin(); i != _targets.end(); ++i) {
		(*i)->_logger.log(**i, msg, _prefix);
	}
}

Log& Log::error()
{
	static Log log(ISL__ERROR_LOG_PREFIX);
	return log;
}

Log& Log::warning()
{
	static Log log(ISL__WARNING_LOG_PREFIX);
	return log;
}

Log& Log::debug()
{
	static Log log(ISL__DEBUG_LOG_PREFIX);
	return log;
}

} // namespace isl

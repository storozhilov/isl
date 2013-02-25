#include <isl/DirectLogger.hxx>
#include <isl/AbstractLogTarget.hxx>

namespace isl
{

DirectLogger::DirectLogger() :
	AbstractLogger(),
	_targets()
{}

DirectLogger::~DirectLogger()
{
	for (TargetsContainer::iterator i = _targets.begin(); i != _targets.end(); ++i) {
		delete i->second;
	}
}

void DirectLogger::registerTarget(AbstractLogTarget& target)
{
	if (_targets.find(&target) != _targets.end()) {
		// TODO
		return;
	}
	_targets.insert(TargetsContainer::value_type(&target, new Mutex()));
}

void DirectLogger::unregisterTarget(AbstractLogTarget& target)
{
	TargetsContainer::iterator pos = _targets.find(&target);
	if (pos == _targets.end()) {
		// TODO
		return;
	}
	delete pos->second;
	_targets.erase(pos);
}

void DirectLogger::log(AbstractLogTarget& target, const AbstractLogMessage& msg, const std::string& prefix)
{
	TargetsContainer::iterator pos = _targets.find(&target);
	if (pos == _targets.end()) {
		// TODO
		return;
	}
	MutexLocker locker(*(pos->second));
	pos->first->log(msg, prefix);
}

} // namespace isl

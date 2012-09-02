#include <isl/SubsystemThread.hxx>
#include <isl/AbstractSubsystem.hxx>

namespace isl
{

SubsystemThread::SubsystemThread(AbstractSubsystem& subsystem, bool autoStop, bool awaitStartup) :
	Thread(awaitStartup),
	_subsystem(subsystem),
	_autoStop(autoStop),
	_shouldTerminate(false),
	_shouldTerminateCond()
{
	_subsystem.registerThread(this);
}

SubsystemThread::~SubsystemThread()
{
	_subsystem.unregisterThread(this);
}

bool SubsystemThread::shouldTerminate()
{
	MutexLocker locker(_shouldTerminateCond.mutex());
	return _shouldTerminate;
}

bool SubsystemThread::awaitShouldTerminate(Timeout timeout)
{
	MutexLocker locker(_shouldTerminateCond.mutex());
	if (_shouldTerminate) {
		return true;
	}
	_shouldTerminateCond.wait(timeout);
	return _shouldTerminate;
}

void SubsystemThread::setShouldTerminate(bool newValue)
{
	MutexLocker locker(_shouldTerminateCond.mutex());
	_shouldTerminate = newValue;
	_shouldTerminateCond.wakeAll();
}

} // namespace isl

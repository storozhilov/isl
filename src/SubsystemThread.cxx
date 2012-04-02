#include <isl/SubsystemThread.hxx>
#include <isl/AbstractSubsystem.hxx>

namespace isl
{

SubsystemThread::SubsystemThread(AbstractSubsystem& subsystem, bool awaitStartup) :
	Thread(awaitStartup),
	_subsystem(subsystem)
{
	_subsystem.registerThread(this);
}

SubsystemThread::~SubsystemThread()
{
	_subsystem.unregisterThread(this);
}

bool SubsystemThread::shouldTerminate()
{
	MutexLocker locker(_subsystem._stateCond.mutex());
	return _subsystem._state == AbstractSubsystem::StoppingState || _subsystem._state == AbstractSubsystem::IdlingState;
}

bool SubsystemThread::awaitTermination(Timeout timeout)
{
	MutexLocker locker(_subsystem._stateCond.mutex());
	if (_subsystem._state == AbstractSubsystem::StoppingState || _subsystem._state == AbstractSubsystem::IdlingState) {
		return true;
	}
	_subsystem._stateCond.wait(timeout);
	return _subsystem._state == AbstractSubsystem::StoppingState || _subsystem._state == AbstractSubsystem::IdlingState;
}

} // namespace isl

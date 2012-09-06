#include <isl/Subsystem.hxx>

namespace isl
{

//------------------------------------------------------------------------------
// Subsystem
//------------------------------------------------------------------------------

Subsystem::Subsystem(Subsystem * owner) :
	runtimeParamsRWLock(),
	_owner(owner),
	_children(),
	_threads(),
	_startStopMutex()
{
	if (_owner) {
		_owner->registerChild(this);
	}
}

Subsystem::~Subsystem()
{
	if (_owner) {
		_owner->unregisterChild(this);
	}
	for (Children::iterator i = _children.begin(); i != _children.end(); ++i) {
		// TODO Warning log message?
		(*i)->_owner = 0;
	}
}

void Subsystem::start()
{
	MutexLocker locker(_startStopMutex);
	beforeStart();
	startImpl();
	afterStart();
}

void Subsystem::stop()
{
	MutexLocker locker(_startStopMutex);
	beforeStop();
	stopImpl();
	afterStop();
}

void Subsystem::startImpl()
{
	// Starting children subsystems
	for (Children::iterator i = _children.begin(); i != _children.end(); ++i) {
		(*i)->start();
	}
	// Starting threads
	for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
		(*i)->setShouldTerminate(false);
		if ((*i)->autoStart()) {
			(*i)->start();
		}
	}
}

void Subsystem::stopImpl()
{
	// Stopping threads (TODO Timed join & killing thread if it has not been terminated)
	for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
		if ((*i)->autoStop()) {
			(*i)->setShouldTerminate(true);
			(*i)->join();
		}
	}
	// Stopping chldren subsystems
	for (Children::iterator i = _children.begin(); i != _children.end(); ++i) {
		(*i)->stop();
	}
}

void Subsystem::registerChild(Subsystem * child)
{
	if (std::find(_children.begin(), _children.end(), child) != _children.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Child subsystem has been already registered in subsystem"));
	}
	_children.push_back(child);
}

void Subsystem::unregisterChild(Subsystem * child)
{
	Children::iterator childPos = std::find(_children.begin(), _children.end(), child);
	if (childPos == _children.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Child subsystem have not been registered in subsystem"));
	}
	_children.erase(childPos);
}

void Subsystem::registerThread(Subsystem::AbstractThread * thread)
{
	if (std::find(_threads.begin(), _threads.end(), thread) != _threads.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread has been already registered in subsystem"));
	}
	_threads.push_back(thread);
}

void Subsystem::unregisterThread(Subsystem::AbstractThread * thread)
{
	Threads::iterator threadPos = std::find(_threads.begin(), _threads.end(), thread);
	if (threadPos == _threads.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Child subsystem have not been registered in subsystem"));
	}
	_threads.erase(threadPos);
}

//------------------------------------------------------------------------------
// Subsystem::AbstractThread
//------------------------------------------------------------------------------

Subsystem::AbstractThread::AbstractThread(Subsystem& subsystem, bool autoStart, bool autoStop, bool isTrackable, bool awaitStartup) :
	::isl::AbstractThread(isTrackable, awaitStartup),
	_subsystem(subsystem),
	_autoStart(autoStart),
	_autoStop(autoStop),
	_shouldTerminate(false),
	_shouldTerminateCond()
{
	_subsystem.registerThread(this);
}

Subsystem::AbstractThread::~AbstractThread()
{
	_subsystem.unregisterThread(this);
}

bool Subsystem::AbstractThread::shouldTerminate()
{
	MutexLocker locker(_shouldTerminateCond.mutex());
	return _shouldTerminate;
}

bool Subsystem::AbstractThread::awaitShouldTerminate(Timeout timeout)
{
	MutexLocker locker(_shouldTerminateCond.mutex());
	if (_shouldTerminate) {
		return true;
	}
	_shouldTerminateCond.wait(timeout);
	return _shouldTerminate;
}

void Subsystem::AbstractThread::setShouldTerminate(bool newValue)
{
	MutexLocker locker(_shouldTerminateCond.mutex());
	_shouldTerminate = newValue;
	_shouldTerminateCond.wakeAll();
}

} // namespace isl

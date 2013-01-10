#include <isl/StateSetSubsystem.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/LogMessage.hxx>
#include <isl/common.hxx>
#include <algorithm>

namespace isl
{

//------------------------------------------------------------------------------
// StateSetSubsystem
//------------------------------------------------------------------------------

StateSetSubsystem::StateSetSubsystem(Subsystem * owner, const Timeout& clockTimeout) :
	Subsystem(owner, clockTimeout),
	_stateSet(),
	_threads()
{}

void StateSetSubsystem::start()
{
	_stateSet.resetUnsafe();
	Subsystem::start();
	startThreads();
}

void StateSetSubsystem::stop()
{
	stopThreads();
	Subsystem::stop();
}

void StateSetSubsystem::appointTermination()
{
	_stateSet.insert(TerminationState);
}

void StateSetSubsystem::startThreads()
{
	for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
		(*i)->start();
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Threaded subsystem's thread has been started"));
	}
}

void StateSetSubsystem::stopThreads()
{
	if (_threads.empty()) {
		return;
	}
	appointTermination();
	debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Termination has been appointed"));
	for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Joining a threaded subsystem's thread"));
		(*i)->join();
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Threaded subsystem's thread has been terminated"));
	}
}

void StateSetSubsystem::registerThread(StateSetSubsystem::AbstractThread * thread)
{
	if (std::find(_threads.begin(), _threads.end(), thread) != _threads.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread has been already registered in threaded subsystem"));
	}
	_threads.push_back(thread);
}

void StateSetSubsystem::unregisterThread(StateSetSubsystem::AbstractThread * thread)
{
	Threads::iterator threadPos = std::find(_threads.begin(), _threads.end(), thread);
	if (threadPos == _threads.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread have not been registered in threaded subsystem"));
	}
	_threads.erase(threadPos);
}

//------------------------------------------------------------------------------
// StateSetSubsystem::AbstractThread
//------------------------------------------------------------------------------

StateSetSubsystem::AbstractThread::AbstractThread(StateSetSubsystem& subsystem, bool isTrackable, bool awaitStartup) :
	::isl::AbstractThread(isTrackable, awaitStartup),
	_subsystem(subsystem)
{
	_subsystem.registerThread(this);
}

StateSetSubsystem::AbstractThread::~AbstractThread()
{
	_subsystem.unregisterThread(this);
}

bool StateSetSubsystem::AbstractThread::shouldTerminate()
{
	StateSetType::SetType set = _subsystem.stateSet().fetch();
	return set.find(TerminationState) != set.end();
}

bool StateSetSubsystem::AbstractThread::awaitTermination(const Timestamp& limit)
{
	std::auto_ptr<StateSetType::SetType> setAutoPtr = _subsystem.stateSet().await(TerminationState, limit);
	return setAutoPtr.get() ? (setAutoPtr->find(TerminationState) != setAutoPtr->end()) : false;
}

bool StateSetSubsystem::AbstractThread::awaitTermination(const Timeout& timeout, Timeout * timeoutLeft)
{
	Timestamp limit = Timestamp::limit(timeout);
	bool result = awaitTermination(limit);
	if (timeoutLeft) {
		*timeoutLeft = result ? limit.leftTo() : Timeout();
	}
	return result;
}

} // namespace isl

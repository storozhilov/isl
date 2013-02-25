#include <isl/StateSetSubsystem.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <isl/Ticker.hxx>
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
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "State set subsystem's thread has been started"));
	}
}

void StateSetSubsystem::stopThreads()
{
	if (_threads.empty()) {
		return;
	}
	appointTermination();
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Termination has been appointed"));
	for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Joining a state set subsystem's thread"));
		(*i)->join();
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "State set subsystem's thread has been terminated"));
	}
}

void StateSetSubsystem::registerThread(StateSetSubsystem::AbstractThread * thread)
{
	if (std::find(_threads.begin(), _threads.end(), thread) != _threads.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread has been already registered in state set subsystem"));
	}
	_threads.push_back(thread);
}

void StateSetSubsystem::unregisterThread(StateSetSubsystem::AbstractThread * thread)
{
	Threads::iterator threadPos = std::find(_threads.begin(), _threads.end(), thread);
	if (threadPos == _threads.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread have not been registered in state set subsystem"));
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
	bool shouldTerminate;
	_subsystem.stateSet().await(TerminationState, limit, &shouldTerminate);
	return shouldTerminate;
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

//------------------------------------------------------------------------------
// StateSetSubsystem::Thread
//------------------------------------------------------------------------------

StateSetSubsystem::Thread::Thread(StateSetSubsystem& subsystem, bool isTrackable, bool awaitStartup) :
	StateSetSubsystem::AbstractThread(subsystem, isTrackable, awaitStartup)
{}

void StateSetSubsystem::Thread::run()
{
	if (!onStart()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "State set subsystem thread has been terminated by onStart() event handler -> exiting from the thread execution"));
		return;
	}
	StateSetType::SetType currentStateSet = subsystem()._stateSet.fetch();
	Ticker ticker(subsystem().clockTimeout());
	bool firstTick = true;
	while (true) {
		size_t ticksExpired;
		Timestamp nextTickLimit = ticker.tick(&ticksExpired);
		if (firstTick) {
			firstTick = false;
			if (shouldTerminate()) {
				// Termination state has been detected
				Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "State set subsystem termination has been detected on first tick -> exiting from the thread execution"));
				break;
			}
		} else if (ticksExpired > 1) {
			// Overload has been detected
			Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "State set subsystem thread execution overload has been detected: ") << ticksExpired << " ticks expired");
			if (!onOverload(ticksExpired, currentStateSet)) {
				Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "State set subsystem thread has been terminated by onOverload() event handler -> exiting from the thread execution"));
			}
		}
		// Doing the job
		if (!doLoad(nextTickLimit, currentStateSet)) {
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "State set subsystem thread has been terminated by doLoad() method -> exiting from the thread execution"));
			break;
		}
		// Awaiting for the termination
		bool shouldTerminate;
		currentStateSet = subsystem()._stateSet.await(TerminationState, nextTickLimit, &shouldTerminate);
		if (shouldTerminate) {
			// Termination state has been detected
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "State set subsystem termination has been detected -> exiting from the thread execution"));
			break;
		}
	}
	onStop();
}

} // namespace isl

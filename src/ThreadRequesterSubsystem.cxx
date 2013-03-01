#include <isl/ThreadRequesterSubsystem.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <isl/Ticker.hxx>
#include <algorithm>

namespace isl
{


//------------------------------------------------------------------------------
// ThreadRequesterSubsystem
//------------------------------------------------------------------------------

ThreadRequesterSubsystem::ThreadRequesterSubsystem(Subsystem * owner, const Timeout& clockTimeout, const Timeout& awaitResponseTimeout) :
	Subsystem(owner, clockTimeout),
	_awaitResponseTimeout(awaitResponseTimeout),
	_threads()
{}

void ThreadRequesterSubsystem::start()
{
	// Calling ancestor's method
	Subsystem::start();
	// Starting threads
	for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
		(*i)->_shouldTerminate = false;
		(*i)->requester().reset();
		(*i)->start();
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Thread requester subsystem's thread has been started"));
	}
}

void ThreadRequesterSubsystem::stop()
{
	// Sending termination request to all threads
	std::map<AbstractThread *, size_t> terminationRequestsMap;
	for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
		size_t requestId = (*i)->requester().sendRequest(TerminationRequest());
		if (requestId <= 0) {
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Could not send termination request to the thread requester subsystem's thread"));
		} else {
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Termination request has been sent to the thread requester subsystem's thread"));
		}
		terminationRequestsMap[*i] = requestId;
	}
	Timestamp limit = Timestamp::limit(_awaitResponseTimeout);
	// Receiving responses to the termination requests from all threads and joining them
	for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
		if (terminationRequestsMap[*i] <= 0) {
			// TODO (*i)->kill();
			continue;
		}
		// Receiving response to the termination request from the thread
		std::auto_ptr<AbstractThreadMessage> responseAutoPtr = (*i)->requester().awaitResponse(terminationRequestsMap[*i], limit);
		if (!responseAutoPtr.get()) {
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "No response to termination request have been received from the thread requester subsystem's thread"));
		} else if (responseAutoPtr->instanceOf<OkResponse>()) {
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "OK response to the termination request has been received from the thread requester subsystem's thread"));
		} else {
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Invalid response to termination request has been received from the sender thread: \"") <<
					responseAutoPtr->name() << '"');
		}
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Joining a thread requester subsystem's thread"));
		(*i)->join();
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Thread requester subsystem's thread has been terminated"));
	}
	// Calling ancestor's method
	Subsystem::stop();
}

void ThreadRequesterSubsystem::registerThread(ThreadRequesterSubsystem::AbstractThread * thread)
{
	if (std::find(_threads.begin(), _threads.end(), thread) != _threads.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread has been already registered in thread requester subsystem"));
	}
	_threads.push_back(thread);
}

void ThreadRequesterSubsystem::unregisterThread(ThreadRequesterSubsystem::AbstractThread * thread)
{
	Threads::iterator threadPos = std::find(_threads.begin(), _threads.end(), thread);
	if (threadPos == _threads.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread have not been registered in thread requester subsystem"));
	}
	_threads.erase(threadPos);
}

//------------------------------------------------------------------------------
// ThreadRequesterSubsystem::AbstractThread
//------------------------------------------------------------------------------

ThreadRequesterSubsystem::AbstractThread::AbstractThread(ThreadRequesterSubsystem& subsystem, bool isTrackable, bool awaitStartup) :
	::isl::AbstractThread(isTrackable, awaitStartup),
	_subsystem(subsystem),
	_requester(),
	_shouldTerminate()
{
	_subsystem.registerThread(this);
}

ThreadRequesterSubsystem::AbstractThread::~AbstractThread()
{
	_subsystem.unregisterThread(this);
}

void ThreadRequesterSubsystem::AbstractThread::processThreadRequests()
{
	while (const ThreadRequesterType::PendingRequest * pendingRequestPtr = _requester.fetchRequest()) {
		processThreadRequest(*pendingRequestPtr);
	}
}

void ThreadRequesterSubsystem::AbstractThread::processThreadRequests(const Timestamp& limit)
{
	while (const ThreadRequesterType::PendingRequest * pendingRequestPtr = _requester.awaitRequest(limit)) {
		processThreadRequest(*pendingRequestPtr);
	}
}

void ThreadRequesterSubsystem::AbstractThread::processThreadRequest(const ThreadRequesterType::PendingRequest& pendingRequest)
{
	if (pendingRequest.request().instanceOf<TerminationRequest>()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
					"Termination request has been received by the thread requester subsystem thread -> setting the termination flag to TRUE"));
		_shouldTerminate = true;
		if (pendingRequest.responseRequired()) {
			_requester.sendResponse(OkResponse());
		}
	}
	onThreadRequest(pendingRequest);
}

//------------------------------------------------------------------------------
// ThreadRequesterSubsystem::Thread
//------------------------------------------------------------------------------

ThreadRequesterSubsystem::Thread::Thread(ThreadRequesterSubsystem& subsystem, bool isTrackable, bool awaitStartup) :
	ThreadRequesterSubsystem::AbstractThread(subsystem, isTrackable, awaitStartup)
{}

void ThreadRequesterSubsystem::Thread::run()
{
	if (!onStart()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Thread requester subsystem thread has been terminated by onStart() event handler -> exiting from the thread execution"));
		return;
	}
	Ticker ticker(subsystem().clockTimeout());
	while (true) {
		size_t ticksExpired;
		Timestamp nextTickLimit = ticker.tick(&ticksExpired);
		if (ticksExpired > 1) {
			// Overload has been detected
			Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Thread requester subsystem thread execution overload has been detected: ") << ticksExpired << " ticks expired");
			if (!onOverload(ticksExpired)) {
				Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
							"Thread requester subsystem thread has been terminated by onOverload() event handler -> exiting from the thread execution"));
			}
		}
		// Doing the job
		if (!doLoad(nextTickLimit)) {
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Thread requester subsystem thread has been terminated by doLoad() method -> exiting from the thread execution"));
			break;
		}
		// Awaiting for requests and processing them
		processThreadRequests(nextTickLimit);
		if (shouldTerminate()) {
			// Termination state has been detected
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Thread requester subsystem termination has been detected -> exiting from the thread execution"));
			break;
		}
	}
	onStop();
}

} // namespace isl

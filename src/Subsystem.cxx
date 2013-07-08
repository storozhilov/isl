#include <isl/Subsystem.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <isl/Ticker.hxx>
#include <algorithm>

namespace isl
{

//------------------------------------------------------------------------------
// Subsystem
//------------------------------------------------------------------------------

Subsystem::Subsystem(Subsystem * owner, const Timeout& clockTimeout, size_t awaitResponseTicksAmount) :
	_owner(owner),
	_clockTimeout(clockTimeout),
	_awaitResponseTicksAmount(awaitResponseTicksAmount),
	_children(),
	_threads()
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

void Subsystem::startChildren()
{
	for (Children::iterator i = _children.begin(); i != _children.end(); ++i) {
		(*i)->start();
	}
}

void Subsystem::stopChildren()
{
	for (Children::iterator i = _children.begin(); i != _children.end(); ++i) {
		(*i)->stop();
	}
}

void Subsystem::start()
{
	startThreads();
	startChildren();
}

void Subsystem::stop()
{
	stopChildren();
	stopThreads();
}

void Subsystem::startThreads()
{
	for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
		(*i)->start();
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Subsystem's thread has been started"));
	}
}

void Subsystem::stopThreads()
{
	if (_threads.empty()) {
		return;
	}
	for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
		(*i)->appointTermination();
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Termination has been appointed to subsystem's thread"));
		// TODO: Timed join for _awaitResponseTicksAmount ticks and kill if it does not stop
		(*i)->thread().join();
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Subsystem's thread has been terminated"));
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
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread have not been registered in subsystem"));
	}
	_threads.erase(threadPos);
}

//------------------------------------------------------------------------------
// Subsystem::AbstractRequesterThread
//------------------------------------------------------------------------------

Subsystem::AbstractRequesterThread::AbstractRequesterThread(Subsystem& subsystem, bool isTrackable, bool awaitStartup) :
	AbstractThread(subsystem, isTrackable, awaitStartup),
	_requester(),
	_shouldTerminate()
{}

Subsystem::AbstractRequesterThread::~AbstractRequesterThread()
{}

void Subsystem::AbstractRequesterThread::appointTermination()
{
	if (thread().handle() == Thread::self()) {
		_shouldTerminate = true;
		return;
	}
	// Sending termination request
	size_t requestId = _requester.sendRequest(TerminationRequest());
	if (requestId <= 0) {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Could not send termination request to the thread requester thread"));
	} else {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Termination request has been sent to the thread requester thread"));
	}
	// Fetching the response
	Timestamp limit = Timestamp::limit(subsystem().clockTimeout() * subsystem().awaitResponseTicksAmount());
	std::auto_ptr<AbstractThreadMessage> responseAutoPtr = _requester.awaitResponse(requestId, limit);
	if (!responseAutoPtr.get()) {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS,
					"No response to termination request have been received from the thread requester thread"));
	} else if (responseAutoPtr->instanceOf<OkResponse>()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
					"OK response to the termination request has been received from the thread requester thread"));
	} else {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS,
					"Invalid response to termination request has been received from the thread requester thread: \"") <<
				responseAutoPtr->name() << '"');
	}
}

void Subsystem::AbstractRequesterThread::processRequests()
{
	while (const ThreadRequesterType::PendingRequest * pendingRequestPtr = _requester.fetchRequest()) {
		processThreadRequest(*pendingRequestPtr);
	}
}

void Subsystem::AbstractRequesterThread::processRequests(const Timestamp& limit)
{
	while (const ThreadRequesterType::PendingRequest * pendingRequestPtr = _requester.awaitRequest(limit)) {
		processThreadRequest(*pendingRequestPtr);
	}
}

void Subsystem::AbstractRequesterThread::processThreadRequest(const ThreadRequesterType::PendingRequest& pendingRequest)
{
	if (pendingRequest.request().instanceOf<TerminationRequest>()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
					"Termination request has been received by the thread requester thread -> setting the termination flag to TRUE"));
		_shouldTerminate = true;
		if (pendingRequest.responseRequired()) {
			_requester.sendResponse(OkResponse());
		}
	} else if (pendingRequest.request().instanceOf<PingRequest>()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
					"Ping request has been received by the thread requester thread -> responding with the pong response"));
		if (pendingRequest.responseRequired()) {
			_requester.sendResponse(PongResponse());
		}
	}
	onRequest(pendingRequest);
}

//------------------------------------------------------------------------------
// Subsystem::RequesterThread
//------------------------------------------------------------------------------

Subsystem::RequesterThread::RequesterThread(Subsystem& subsystem, bool isTrackable, bool awaitStartup) :
	Subsystem::AbstractRequesterThread(subsystem, isTrackable, awaitStartup)
{}

void Subsystem::RequesterThread::run()
{
	if (!onStart()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Requester thread thread has been terminated by onStart() event handler -> exiting from the thread execution"));
		return;
	}
	Ticker ticker(subsystem().clockTimeout());
	while (true) {
		size_t ticksExpired;
		Timestamp prevTickTimestamp;
		Timestamp nextTickTimestamp = ticker.tick(&ticksExpired, &prevTickTimestamp);
		if (ticksExpired > 1) {
			// Overload has been detected
			Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Requester thread thread execution overload has been detected: ") << ticksExpired << " ticks expired");
			if (!onOverload(prevTickTimestamp, nextTickTimestamp, ticksExpired)) {
				Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
							"Requester thread thread has been terminated by onOverload() event handler -> exiting from the thread execution"));
				break;
			}
		}
		// Doing the job
		if (!doLoad(prevTickTimestamp, nextTickTimestamp, ticksExpired)) {
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Requester thread thread has been terminated by doLoad() method -> exiting from the thread execution"));
			break;
		}
		// Awaiting for requests and processing them
		processRequests(nextTickTimestamp);
		if (shouldTerminate()) {
			// Termination state has been detected
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Requester thread termination has been detected -> exiting from the thread execution"));
			break;
		}
	}
	onStop();
}

} // namespace isl

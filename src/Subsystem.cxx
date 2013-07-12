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

std::auto_ptr<Subsystem::ThreadRequesterType::MessageType> Subsystem::AbstractRequesterThread::sendRequest(
		const ThreadRequesterType::MessageType& request, const Timestamp& awaitResponseLimit)
{
	if (thread().handle() == Thread::self()) {
		// Process request directly if called from inside
		return processRequest(request, true);
	}
	// Sending termination request
	size_t requestId = _requester.sendRequest(request);
	if (requestId <= 0) {
		Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Could not send request to the thread requester thread"));
		return std::auto_ptr<ThreadRequesterType::MessageType>();
	}
	Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Request has been sent to the thread requester thread"));
	// Fetching the response
	return _requester.awaitResponse(requestId, awaitResponseLimit);
}

void Subsystem::AbstractRequesterThread::appointTermination()
{
	std::auto_ptr<ThreadRequesterType::MessageType> responseAutoPtr = sendRequest(TerminationRequest(), Timestamp::limit(subsystem().awaitResponseTimeout()));
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

std::auto_ptr<Subsystem::ThreadRequesterType::MessageType> Subsystem::AbstractRequesterThread::onRequest(const ThreadRequesterType::MessageType& request, bool responseRequired)
{
	Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Unrecognized request: '") << request.name() << '\'');
	return std::auto_ptr<ThreadRequesterType::MessageType>();
}

void Subsystem::AbstractRequesterThread::processRequests()
{
	while (const ThreadRequesterType::PendingRequest * pendingRequestPtr = _requester.fetchRequest()) {
		std::auto_ptr<ThreadRequesterType::MessageType> responseAutoPtr = processRequest(pendingRequestPtr->request(), pendingRequestPtr->responseRequired());
		if (responseAutoPtr.get()) {
			_requester.sendResponse(*responseAutoPtr.get());
		} else if (pendingRequestPtr->responseRequired()) {
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "No response to the request, which requires one"));
		}
	}
}

void Subsystem::AbstractRequesterThread::processRequests(const Timestamp& limit)
{
	while (const ThreadRequesterType::PendingRequest * pendingRequestPtr = _requester.awaitRequest(limit)) {
		std::auto_ptr<ThreadRequesterType::MessageType> responseAutoPtr = processRequest(pendingRequestPtr->request(), pendingRequestPtr->responseRequired());
		if (responseAutoPtr.get()) {
			_requester.sendResponse(*responseAutoPtr.get());
		} else if (pendingRequestPtr->responseRequired()) {
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "No response to the request, which requires one"));
		}
	}
}

std::auto_ptr<Subsystem::ThreadRequesterType::MessageType> Subsystem::AbstractRequesterThread::processRequest(const ThreadRequesterType::MessageType& request, bool responseRequired)
{
	if (request.instanceOf<TerminationRequest>()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
					"Termination request has been received by the thread requester thread -> setting the termination flag to TRUE"));
		_shouldTerminate = true;
		return std::auto_ptr<ThreadRequesterType::MessageType>(responseRequired ? new OkResponse() : 0);
	} else if (request.instanceOf<PingRequest>()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
					"Ping request has been received by the thread requester thread -> responding with the pong response"));
		return std::auto_ptr<ThreadRequesterType::MessageType>(responseRequired ? new PongResponse() : 0);
	}
	return onRequest(request, responseRequired);
}

//------------------------------------------------------------------------------
// Subsystem::RequesterThread
//------------------------------------------------------------------------------

Subsystem::RequesterThread::RequesterThread(Subsystem& subsystem, bool isTrackable, bool awaitStartup) :
	Subsystem::AbstractRequesterThread(subsystem, isTrackable, awaitStartup)
{}

void Subsystem::RequesterThread::run()
{
	onStart();
	Ticker ticker(subsystem().clockTimeout());
	while (true) {
		// Handling termination
		if (shouldTerminate()) {
			// Termination state has been detected
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Requester thread termination has been detected -> exiting from the thread execution"));
			break;
		}
		size_t ticksExpired;
		Timestamp prevTickTimestamp;
		Timestamp nextTickTimestamp = ticker.tick(&ticksExpired, &prevTickTimestamp);
		if (ticksExpired > 1) {
			// Overload has been detected
			Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Requester thread thread execution overload has been detected: ") << ticksExpired << " ticks expired");
			onOverload(prevTickTimestamp, nextTickTimestamp, ticksExpired);
			// Handling termination
			if (shouldTerminate()) {
				// Termination state has been detected
				Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
							"Requester thread termination after overload event triggering has been detected -> exiting from the thread execution"));
				break;
			}
		}
		// Doing the job
		doLoad(prevTickTimestamp, nextTickTimestamp, ticksExpired);
		// Awaiting for requests and processing them
		processRequests(nextTickTimestamp);
	}
	onStop();
}

} // namespace isl

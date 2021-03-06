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
// Subsystem::AbstractRequestableThread
//------------------------------------------------------------------------------

Subsystem::AbstractRequestableThread::AbstractRequestableThread(Subsystem& subsystem, bool isTrackable, bool awaitStartup) :
	AbstractThread(subsystem, isTrackable, awaitStartup),
	_requester(),
	_shouldTerminate(false)
{}

Subsystem::AbstractRequestableThread::~AbstractRequestableThread()
{}

std::auto_ptr<Subsystem::ThreadRequesterType::MessageType> Subsystem::AbstractRequestableThread::sendRequest(
		const ThreadRequesterType::MessageType& request, const Timestamp& awaitResponseLimit)
{
	if (thread().handle() == Thread::self()) {
		// Process request directly if called from inside
                bool stopRequestsProcessing = false;
		return processRequest(request, true, stopRequestsProcessing);
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

void Subsystem::AbstractRequestableThread::appointTermination()
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

std::auto_ptr<Subsystem::ThreadRequesterType::MessageType> Subsystem::AbstractRequestableThread::onRequest(const ThreadRequesterType::MessageType& request, bool responseRequired,
                                bool& stopRequestsProcessing)
{
	Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Unrecognized request: '") << request.name() << '\'');
	return std::auto_ptr<ThreadRequesterType::MessageType>();
}

void Subsystem::AbstractRequestableThread::processRequests()
{
	while (const ThreadRequesterType::PendingRequest * pendingRequestPtr = _requester.fetchRequest()) {
                bool stopRequestsProcessing = false;
		std::auto_ptr<ThreadRequesterType::MessageType> responseAutoPtr = processRequest(pendingRequestPtr->request(), pendingRequestPtr->responseRequired(),
                                stopRequestsProcessing);
		if (responseAutoPtr.get()) {
			_requester.sendResponse(*responseAutoPtr.get());
		} else if (pendingRequestPtr->responseRequired()) {
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "No response to the request, which requires one"));
		}
                if (stopRequestsProcessing) {
                        break;
                }
	}
}

void Subsystem::AbstractRequestableThread::processRequests(const Timestamp& limit)
{
	while (const ThreadRequesterType::PendingRequest * pendingRequestPtr = _requester.awaitRequest(limit)) {
                bool stopRequestsProcessing = false;
		std::auto_ptr<ThreadRequesterType::MessageType> responseAutoPtr = processRequest(pendingRequestPtr->request(), pendingRequestPtr->responseRequired(),
                                stopRequestsProcessing);
		if (responseAutoPtr.get()) {
			_requester.sendResponse(*responseAutoPtr.get());
		} else if (pendingRequestPtr->responseRequired()) {
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "No response to the request, which requires one"));
		}
                if (stopRequestsProcessing) {
                        break;
                }
	}
}

std::auto_ptr<Subsystem::ThreadRequesterType::MessageType> Subsystem::AbstractRequestableThread::processRequest(const ThreadRequesterType::MessageType& request, bool responseRequired,
                                bool& stopRequestsProcessing)
{
	if (request.instanceOf<TerminationRequest>()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
					"Termination request has been received by the thread requester thread -> setting the termination flag to TRUE"));
		_shouldTerminate = true;
                stopRequestsProcessing = false;
		return std::auto_ptr<ThreadRequesterType::MessageType>(responseRequired ? new OkResponse() : 0);
	} else if (request.instanceOf<PingRequest>()) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
					"Ping request has been received by the thread requester thread -> responding with the pong response"));
		return std::auto_ptr<ThreadRequesterType::MessageType>(responseRequired ? new PongResponse() : 0);
	}
	return onRequest(request, responseRequired, stopRequestsProcessing);
}

//------------------------------------------------------------------------------
// Subsystem::OscillatorThread
//------------------------------------------------------------------------------

Subsystem::OscillatorThread::OscillatorThread(Subsystem& subsystem, bool isTrackable, bool awaitStartup) :
	Subsystem::AbstractRequestableThread(subsystem, isTrackable, awaitStartup)
{}

void Subsystem::OscillatorThread::run()
{
	onStart();
	Ticker ticker(subsystem().clockTimeout());
	while (true) {
		// Handling termination
		if (shouldTerminate()) {
			// Termination state has been detected
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Oscillator thread termination has been detected -> exiting from the thread execution"));
			break;
		}
		size_t ticksExpired;
		Timestamp prevTick;
		Timestamp nextTick = ticker.tick(&ticksExpired, &prevTick);
		if (ticksExpired > 1) {
			// Overload has been detected
			Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Oscillator thread execution overload has been detected: ") << ticksExpired << " ticks expired");
			onOverload(prevTick, nextTick, ticksExpired);
			// Handling termination
			if (shouldTerminate()) {
				// Termination state has been detected
				Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
							"Oscillator thread termination after overload event triggering has been detected -> exiting from the thread execution"));
				break;
			}
		}
		// Doing the job
		doLoad(prevTick, nextTick, ticksExpired);
		// Awaiting for requests and processing them
		processRequests(nextTick);
	}
	onStop();
}

//------------------------------------------------------------------------------
// Subsystem::SchedulerThread
//------------------------------------------------------------------------------

Subsystem::SchedulerThread::SchedulerThread(Subsystem& subsystem, bool isTrackable, bool awaitStartup) :
	Subsystem::AbstractRequestableThread(subsystem, isTrackable, awaitStartup),
        _loadScheduled()
{}

void Subsystem::SchedulerThread::run()
{
	onStart();
        _loadScheduled = Timestamp::now();
	while (true) {
		// Handling termination
		if (shouldTerminate()) {
			// Termination state has been detected
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS,
                                                "Scheduler thread termination has been detected -> exiting from the thread execution"));
			break;
		}
                // Executing load if not executed
                Timestamp now = Timestamp::now();
                if (now >= _loadScheduled) {
                        Timestamp limit = now + subsystem().clockTimeout();
                        _loadScheduled = doLoad(now, limit);
                        if (Timestamp::now() >= (limit + subsystem().clockTimeout())) {
                                Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS,
                                                        "Scheduler thread execution overload has been detected"));
                                onOverload(now, limit);
                        }
                }
                // Awaiting for requests until next load cycle
                processRequests(_loadScheduled);
	}
	onStop();
}

} // namespace isl

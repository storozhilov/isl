#include <isl/Subsystem.hxx>

namespace isl
{

//------------------------------------------------------------------------------
// Subsystem
//------------------------------------------------------------------------------

Subsystem::Subsystem(Subsystem * owner, const Timeout& clockTimeout, size_t awaitResponseTimeoutRatio) :
	_owner(owner),
	_clockTimeout(clockTimeout),
	_awaitResponseTimeoutRatio(awaitResponseTimeoutRatio),
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

void Subsystem::startThreads()
{
	for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
		(*i)->requester().reset();
		(*i)->start();
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Subsystem's thread has been started"));
	}
}

void Subsystem::stopThreads()
{
	for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
		size_t requestId = (*i)->requester().sendRequest(TerminateRequestMessage());
		if (requestId > 0) {
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Termination request has been sent to the subsystem's thread"));
			std::auto_ptr<AbstractInterThreadMessage> responseAutoPtr = (*i)->requester().awaitResponse(requestId, awaitResponseTimeout());
			if (!responseAutoPtr.get()) {
				std::ostringstream msg;
				msg << "No response to termination request have been received from the subsystem's thread";
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			} else if (responseAutoPtr->instanceOf<OkResponseMessage>()) {
				debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "OK response to the termination request has been received from the subsystem's thread"));
			} else {
				std::ostringstream msg;
				msg << "Invalid response to termination request has been received from the subsystem's thread: \"" << responseAutoPtr->name() << "\"";
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Joining a subsystem's thread"));
			(*i)->join();
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Subsystem's thread has been terminated"));
		} else {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Could not send termination request to the subsystem's thread"));
			// TODO (*i)->kill();
		}
	}
}

void Subsystem::start()
{
	startChildren();
	startThreads();
}

void Subsystem::stop()
{
	stopThreads();
	stopChildren();
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

Subsystem::AbstractThread::AbstractThread(Subsystem& subsystem, bool isTrackable, bool awaitStartup) :
	::isl::AbstractThread(isTrackable, awaitStartup),
	_subsystem(subsystem),
	_requester()
{
	_subsystem.registerThread(this);
}

Subsystem::AbstractThread::~AbstractThread()
{
	_subsystem.unregisterThread(this);
}

} // namespace isl

#include <isl/Subsystem.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <algorithm>

namespace isl
{

//------------------------------------------------------------------------------
// Subsystem
//------------------------------------------------------------------------------

Subsystem::Subsystem(Subsystem * owner, const Timeout& clockTimeout) :
	_owner(owner),
	_clockTimeout(clockTimeout),
	_children()
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
	startChildren();
}

void Subsystem::stop()
{
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

//------------------------------------------------------------------------------
// Subsystem_NEW`
//------------------------------------------------------------------------------

Subsystem_NEW::Subsystem_NEW(Subsystem_NEW * owner, const Timeout& clockTimeout) :
	_owner(owner),
	_clockTimeout(clockTimeout),
	_children(),
	_threads()
{
	if (_owner) {
		_owner->registerChild(this);
	}
}

Subsystem_NEW::~Subsystem_NEW()
{
	if (_owner) {
		_owner->unregisterChild(this);
	}
	for (Children::iterator i = _children.begin(); i != _children.end(); ++i) {
		// TODO Warning log message?
		(*i)->_owner = 0;
	}
}

void Subsystem_NEW::startChildren()
{
	for (Children::iterator i = _children.begin(); i != _children.end(); ++i) {
		(*i)->start();
	}
}

void Subsystem_NEW::stopChildren()
{
	for (Children::iterator i = _children.begin(); i != _children.end(); ++i) {
		(*i)->stop();
	}
}

void Subsystem_NEW::start()
{
	startThreads();
	startChildren();
}

void Subsystem_NEW::stop()
{
	stopChildren();
	startThreads();
}

void Subsystem_NEW::startThreads()
{
	for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
		(*i)->reset();
		(*i)->start();
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Subsystem's thread has been started"));
	}
}

void Subsystem_NEW::stopThreads()
{
	if (_threads.empty()) {
		return;
	}
	for (Threads::iterator i = _threads.begin(); i != _threads.end(); ++i) {
		(*i)->appointTermination();
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Termination has been appointed to subsystem's thread"));
		// TODO: Timed join and kill?
		(*i)->join();
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Subsystem's thread has been terminated"));
	}
}

void Subsystem_NEW::registerChild(Subsystem_NEW * child)
{
	if (std::find(_children.begin(), _children.end(), child) != _children.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Child subsystem has been already registered in subsystem"));
	}
	_children.push_back(child);
}

void Subsystem_NEW::unregisterChild(Subsystem_NEW * child)
{
	Children::iterator childPos = std::find(_children.begin(), _children.end(), child);
	if (childPos == _children.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Child subsystem have not been registered in subsystem"));
	}
	_children.erase(childPos);
}

void Subsystem_NEW::registerThread(Subsystem_NEW::AbstractThread * thread)
{
	if (std::find(_threads.begin(), _threads.end(), thread) != _threads.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread has been already registered in subsystem"));
	}
	_threads.push_back(thread);
}

void Subsystem_NEW::unregisterThread(Subsystem_NEW::AbstractThread * thread)
{
	Threads::iterator threadPos = std::find(_threads.begin(), _threads.end(), thread);
	if (threadPos == _threads.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Thread have not been registered in subsystem"));
	}
	_threads.erase(threadPos);
}

} // namespace isl

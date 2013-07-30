#include <isl/ScadaTimer.hxx>
#include <isl/ScadaProgram.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>

namespace isl
{

//------------------------------------------------------------------------------
// ScadaTimer
//------------------------------------------------------------------------------

ScadaTimer::ScadaTimer(Subsystem * owner, const Timeout& clockTimeout, size_t maxScheduledTasksAmount) :
	Timer(owner, clockTimeout, maxScheduledTasksAmount),
	_scadaTimerThreadPtr(0),
	_programs(),
	_isRunningRwLock(),
	_isRunning(false)
{}

void ScadaTimer::registerProgram(ScadaProgram& program)
{
	if (_programs.find(&program) != _programs.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "SCADA program has been already registered in SCADA timer"));
	}
	_programs.insert(&program);
}

void ScadaTimer::unregisterProgram(ScadaProgram& program)
{
	if (_programs.find(&program) == _programs.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "SCADA program is not registered in SCADA timer"));
	}
	_programs.erase(&program);
}

//------------------------------------------------------------------------------
// ScadaTimer::ScadaProgramMessageEnvelope
//------------------------------------------------------------------------------

ScadaTimer::ScadaProgramMessageEnvelope::ScadaProgramMessageEnvelope(ScadaProgram& program, const AbstractThreadMessage& message) :
	AbstractThreadMessage("SCADA program message envelope"),
	_program(&program),
	_messageAutoPtr(message.clone())
{}

ScadaTimer::ScadaProgramMessageEnvelope::ScadaProgramMessageEnvelope(const ScadaProgramMessageEnvelope& other) :
	AbstractThreadMessage("SCADA program message envelope"),
	_program(other._program),
	_messageAutoPtr(other._messageAutoPtr->clone())
{}

ScadaTimer::ScadaProgramMessageEnvelope& ScadaTimer::ScadaProgramMessageEnvelope::operator=(const ScadaProgramMessageEnvelope& rhs)
{
	this->_program = rhs._program;
	this->_messageAutoPtr.reset(rhs._messageAutoPtr->clone());
	return *this;
}

//------------------------------------------------------------------------------
// ScadaTimer::ScadaTimerThread
//------------------------------------------------------------------------------

ScadaTimer::ScadaTimerThread::ScadaTimerThread(ScadaTimer& scadaTimer) :
	TimerThread(scadaTimer),
	_scadaTimer(scadaTimer)
{}

void ScadaTimer::ScadaTimerThread::onStart()
{
	TimerThread::onStart();
	{
		WriteLocker locker(_scadaTimer._isRunningRwLock);
		_scadaTimer._isRunning = true;
	}
	// TODO: Call onStart() event handler from all programs
}

void ScadaTimer::ScadaTimerThread::onStop()
{
	TimerThread::onStop();
	// TODO: Call onStop() event handler from all programs
	{
		WriteLocker locker(_scadaTimer._isRunningRwLock);
		_scadaTimer._isRunning = false;
	}
}

std::auto_ptr<Subsystem::ThreadRequesterType::MessageType> ScadaTimer::ScadaTimerThread::onRequest(const ThreadRequesterType::MessageType& request, bool responseRequired)
{
	if (request.instanceOf<ScadaProgramMessageEnvelope>()) {
		// TODO: Call onRequest() event handler from the target programs
		const ScadaProgramMessageEnvelope * envPtr = request.cast<const ScadaProgramMessageEnvelope>();
		return envPtr->_program->onRequest(*(envPtr->_messageAutoPtr.get()), responseRequired);
	} else {
		return TimerThread::onRequest(request, responseRequired);
	}
}

} // namespace isl

#include <isl/ScadaProgram.hxx>
#include <isl/ScadaTimer.hxx>

namespace isl
{

ScadaProgram::ScadaProgram(ScadaTimer& scadaTimer) :
	_scadaTimer(scadaTimer)
{
	_scadaTimer.registerProgram(*this);
}

ScadaProgram::~ScadaProgram()
{
	// TODO: Handle exception
	_scadaTimer.unregisterProgram(*this);
}

/*std::auto_ptr<Subsystem::ThreadRequesterType::MessageType> ScadaProgram::sendRequest(const Subsystem::ThreadRequesterType::MessageType& request,
		const Timestamp& awaitResponseLimit)
{
	ReadLocker locker(_scadaTimer._isRunningRwLock);
	if (!_scadaTimer._isRunning) {
		Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Sending request to not running SCADA program has been detected"));
		return std::auto_ptr<Subsystem::ThreadRequesterType::MessageType>();
	}
	return _scadaTimer._scadaTimerThreadPtr->sendRequest(ScadaTimer::ScadaProgramMessageEnvelope(*this, request), awaitResponseLimit);
}*/

std::auto_ptr<Subsystem::AbstractThreadMessage> ScadaProgram::sendRequest(const Subsystem::AbstractThreadMessage& request,
		const Timestamp& awaitResponseLimit)
{
	ReadLocker locker(_scadaTimer._isRunningRwLock);
	if (!_scadaTimer._isRunning) {
		Log::warning().log(LogMessage(SOURCE_LOCATION_ARGS, "Sending request to not running SCADA program has been detected"));
		return std::auto_ptr<Subsystem::AbstractThreadMessage>();
	}
	return _scadaTimer._scadaTimerThreadPtr->sendRequest(ScadaTimer::ScadaProgramMessageEnvelope(*this, request), awaitResponseLimit);
}

} // namespace isl

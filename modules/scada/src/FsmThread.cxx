#include <isl/FsmThread.hxx>

namespace isl
{

FsmThread::FsmThread(Subsystem& subsystem, AbstractState& initialState, bool isTrackable, bool awaitStartup) :
        Subsystem::AbstractRequestableThread(subsystem, isTrackable, awaitStartup),
        _initialState(initialState),
        _nextStepStatePtr(0),
        _nextStepTimestamp()
{}

void FsmThread::appointNextState(const NextStep& nextStep)
{
        _nextStepStatePtr = &nextStep.first;
        _nextStepTimestamp = Timestamp::now() + nextStep.second;
}

void FsmThread::run()
{
        _nextStepTimestamp = Timestamp::now();
        _nextStepStatePtr = &_initialState;
	onStart();
	while (true) {
		// Handling termination
		if (shouldTerminate()) {
			// Termination state has been detected
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Finite-State Machine thread termination has been detected -> exiting from the thread execution"));
			break;
		}
                // Checking next step timestamp is expired
                Timestamp now = Timestamp::now();
                if (now >= _nextStepTimestamp) {
                        NextStep nextStep = _nextStepStatePtr->makeStep(*this);
                        _nextStepStatePtr = &nextStep.first;
                        _nextStepTimestamp = now + nextStep.second;
                        processRequests();
                } else {
                        processRequests(_nextStepTimestamp);
                }
	}
	onStop();
}

} // namespace isl

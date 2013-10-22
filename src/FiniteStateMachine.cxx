#include <isl/FiniteStateMachine.hxx>

namespace isl
{

//------------------------------------------------------------------------------
// FiniteStateMachine::OscillatorThread
//------------------------------------------------------------------------------

FiniteStateMachine::OscillatorThread::OscillatorThread(Subsystem& subsystem, AbstractState& initialState,
                bool isTrackable, bool awaitStartup) :
        Subsystem::OscillatorThread(subsystem, isTrackable, awaitStartup),
        _initialState(initialState),
        _nextStepStatePtr(0)
{} 

void FiniteStateMachine::OscillatorThread::appointState(AbstractState& state)
{
        _nextStepStatePtr = &state;
}

void FiniteStateMachine::OscillatorThread::onStart()
{
        _nextStepStatePtr = &_initialState;
}

void FiniteStateMachine::OscillatorThread::doLoad(const Timestamp& prevTick, const Timestamp& nextTick, size_t ticksExpired)
{
        while (Timestamp::now() < nextTick) {
                _nextStepStatePtr = &_nextStepStatePtr->makeStep(nextTick);
        }
}

//------------------------------------------------------------------------------
// FiniteStateMachine::SchedulerThread
//------------------------------------------------------------------------------

FiniteStateMachine::SchedulerThread::SchedulerThread(Subsystem& subsystem, AbstractState& initialState,
                bool isTrackable, bool awaitStartup) :
        Subsystem::SchedulerThread(subsystem, isTrackable, awaitStartup),
        _initialState(initialState),
        _nextStepStatePtr(0)
{} 

void FiniteStateMachine::SchedulerThread::scheduleStep(AbstractState& state, const Timestamp& timestamp)
{
        _nextStepStatePtr = &state;
        scheduleLoad(timestamp);
}

void FiniteStateMachine::SchedulerThread::onStart()
{
        _nextStepStatePtr = &_initialState;
}

Timestamp FiniteStateMachine::SchedulerThread::doLoad(const Timestamp& start, const Timestamp& limit)
{
        NextStep nextStep = _nextStepStatePtr->makeStep(limit);
        _nextStepStatePtr = &nextStep.first;
        return start + nextStep.second;
}

} // namespace isl

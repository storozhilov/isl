#include <isl/AbstractSubsystem.hxx>
#include <isl/SubsystemError.hxx>
#include <isl/Core.hxx>

namespace isl
{

const wchar_t AbstractSubsystem::NotDefinedStateName[] = L"<Not defined>";
const wchar_t AbstractSubsystem::IdlingStateName[] = L"Idling";
const wchar_t AbstractSubsystem::StartingStateName[] = L"Starting";
const wchar_t AbstractSubsystem::RunningStateName[] = L"Running";
const wchar_t AbstractSubsystem::StoppingStateName[] = L"Stopping";

} // namespace isl

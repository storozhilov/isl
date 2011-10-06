#include <isl/AbstractSubsystem.hxx>
#include <isl/SubsystemError.hxx>
#include <isl/Core.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractSubsystem
------------------------------------------------------------------------------*/

AbstractSubsystem::AbstractSubsystem(AbstractSubsystem * owner) :
	_owner(owner),
	_state(IdlingState()),
	_stateCond(),
	_commandMutex()
{}

AbstractSubsystem::~AbstractSubsystem()
{}

void AbstractSubsystem::onCommand(const Command& cmd)
{
	if (cmd.equals<StartCommand>()) {
		onStartCommand();
	} else if (cmd.equals<StopCommand>()) {
		onStopCommand();
	} else if (cmd.equals<RestartCommand>()) {
		onRestartCommand();
	} else {
		Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Command '" + cmd.value().name() + L"' is not supported"));
	}
}

} // namespace isl

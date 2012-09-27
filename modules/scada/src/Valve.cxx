#include <isl/Valve.hxx>
#include <isl/common.hxx>
#include <isl/LogMessage.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>

namespace isl
{

Valve::Valve(ModbusEndpoint& endpoint, int openDriveStateBitAddr, int closeDriveStateBitAddr, int openLimitSwitchStateBitAddr,
		int closeLimitSwitchStateBitAddr, int maxOpenDurationRegisterAddr, int maxCloseDurationRegisterAddr,
		int openPowerOffReasonRegisterAddr, int closePowerOffReasonRegisterAddr) :
	_endpoint(endpoint),
	_openDriveStateBitAddr(openDriveStateBitAddr),
	_closeDriveStateBitAddr(closeDriveStateBitAddr),
	_openLimitSwitchStateBitAddr(openLimitSwitchStateBitAddr),
	_closeLimitSwitchStateBitAddr(closeLimitSwitchStateBitAddr),
	_maxOpenDurationRegisterAddr(maxOpenDurationRegisterAddr),
	_maxCloseDurationRegisterAddr(maxCloseDurationRegisterAddr),
	_openPowerOffReasonRegisterAddr(openPowerOffReasonRegisterAddr),
	_closePowerOffReasonRegisterAddr(closePowerOffReasonRegisterAddr)
{}

bool Valve::openDriveState() const
{
	std::vector<uint8_t> bits = _endpoint.readBits(_openDriveStateBitAddr, 1);
	return bits[0];
}

void Valve::setOpenDriveState(bool newValue)
{
	_endpoint.writeBit(_openDriveStateBitAddr, newValue);
}

bool Valve::closeDriveState() const
{
	std::vector<uint8_t> bits = _endpoint.readBits(_closeDriveStateBitAddr, 1);
	return bits[0];
}

void Valve::setCloseDriveState(bool newValue)
{
	_endpoint.writeBit(_closeDriveStateBitAddr, newValue);
}

bool Valve::openLimitSwitchState() const
{
	std::vector<uint8_t> bits = _endpoint.readInputBits(_openLimitSwitchStateBitAddr, 1);
	return bits[0];
}

bool Valve::closeLimitSwitchState() const
{
	std::vector<uint8_t> bits = _endpoint.readInputBits(_closeLimitSwitchStateBitAddr, 1);
	return bits[0];
}

Timeout Valve::maxOpenDuration() const
{
	std::vector<uint16_t> registers = _endpoint.readRegisters(_maxOpenDurationRegisterAddr, 1);
	return Timeout(registers[0] / 10, registers[0] * 100000000);
}

void Valve::setMaxOpenDuration(const Timeout& newValue)
{
	_endpoint.writeRegister(_maxOpenDurationRegisterAddr, newValue.seconds() * 10 + newValue.nanoSeconds() / 100000000);
}

Timeout Valve::maxCloseDuration() const
{
	std::vector<uint16_t> registers = _endpoint.readRegisters(_maxCloseDurationRegisterAddr, 1);
	return Timeout(registers[0] / 10, registers[0] * 100000000);
}

void Valve::setMaxCloseDuration(const Timeout& newValue)
{
	_endpoint.writeRegister(_maxCloseDurationRegisterAddr, newValue.seconds() * 10 + newValue.nanoSeconds() / 100000000);
}

Valve::PowerOffReason Valve::openPowerOffReason() const
{
	std::vector<uint16_t> registers = _endpoint.readInputRegisters(_openPowerOffReasonRegisterAddr, 1);
	return (registers[0] == LimitSwitchRaeson || registers[0] == TimeoutExpiredReason || registers[0] == StopCommandReason || registers[0] == ReverseCommandReason) ?
		static_cast<PowerOffReason>(registers[0]) : UndefinedReason;
}

Valve::PowerOffReason Valve::closePowerOffReason() const
{
	std::vector<uint16_t> registers = _endpoint.readInputRegisters(_closePowerOffReasonRegisterAddr, 1);
	return (registers[0] == LimitSwitchRaeson || registers[0] == TimeoutExpiredReason || registers[0] == StopCommandReason || registers[0] == ReverseCommandReason) ?
		static_cast<PowerOffReason>(registers[0]) : UndefinedReason;
}

} // namespace isl

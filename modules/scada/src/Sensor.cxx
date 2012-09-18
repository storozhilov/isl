#include <isl/Sensor.hxx>

namespace isl
{

Sensor::Sensor(ModbusDevice& device, int valueRegisterAddr) :
	_device(device),
	_valueRegisterAddr(valueRegisterAddr)
{}

uint16_t Sensor::value() const
{
	std::vector<uint16_t> registers = _device.readInputRegisters(_valueRegisterAddr, 1);
	return registers[0];
}

} // namespace isl

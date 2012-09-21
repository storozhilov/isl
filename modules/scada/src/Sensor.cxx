#include <isl/Sensor.hxx>

namespace isl
{

Sensor::Sensor(ModbusEndpoint& endpoint, int valueRegisterAddr) :
	_endpoint(endpoint),
	_valueRegisterAddr(valueRegisterAddr)
{}

uint16_t Sensor::value() const
{
	std::vector<uint16_t> registers = _endpoint.readInputRegisters(_valueRegisterAddr, 1);
	return registers[0];
}

} // namespace isl

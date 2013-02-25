#include <isl/Control.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>

namespace isl
{

Control::Control(ModbusEndpoint& endpoint, int valueRegisterAddr, int feedbackRegisterAddr) :
	_endpoint(endpoint),
	_valueRegisterAddr(valueRegisterAddr),
	_feedbackRegisterAddr(feedbackRegisterAddr)
{}

uint16_t Control::value() const
{
	std::vector<uint16_t> registers = _endpoint.readRegisters(_valueRegisterAddr, 1);
	return registers[0];
}

void Control::setValue(uint16_t newValue)
{
	_endpoint.writeRegister(_valueRegisterAddr, newValue);
}

uint16_t Control::feedbackValue() const
{
	if (_feedbackRegisterAddr <= 0) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "No feedback is provided by the control"));
	}
	std::vector<uint16_t> registers = _endpoint.readInputRegisters(_feedbackRegisterAddr, 1);
	return registers[0];
}

} // namespace isl

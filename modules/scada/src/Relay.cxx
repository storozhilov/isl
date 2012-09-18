#include <isl/Relay.hxx>
#include <isl/common.hxx>
#include <isl/LogMessage.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>

namespace isl
{

Relay::Relay(ModbusDevice& device, int stateBitAddr, int feedbackBitAddr) :
	_device(device),
	_stateBitAddr(stateBitAddr),
	_feedbackBitAddr(feedbackBitAddr)
{}

bool Relay::state() const
{
	std::vector<uint8_t> bits = _device.readBits(_stateBitAddr, 1);
	return bits[0];
}

void Relay::setState(bool newValue)
{
	_device.writeBit(_stateBitAddr, newValue);
}

bool Relay::feedbackState()
{
	if (_feedbackBitAddr <= 0) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Feedback bit address has not been set in relay"));
	}
	std::vector<uint8_t> bits = _device.readInputBits(_feedbackBitAddr, 1);
	return bits[0];
}

} // namespace isl

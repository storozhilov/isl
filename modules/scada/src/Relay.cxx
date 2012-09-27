#include <isl/Relay.hxx>
#include <isl/common.hxx>
#include <isl/LogMessage.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>

namespace isl
{

Relay::Relay(ModbusEndpoint& endpoint, int stateBitAddr, int feedbackBitAddr) :
	_endpoint(endpoint),
	_stateBitAddr(stateBitAddr),
	_feedbackBitAddr(feedbackBitAddr)
{}

bool Relay::state() const
{
	std::vector<uint8_t> bits = _endpoint.readBits(_stateBitAddr, 1);
	return bits[0];
}

void Relay::setState(bool newValue)
{
	_endpoint.writeBit(_stateBitAddr, newValue);
}

bool Relay::feedbackState()
{
	if (_feedbackBitAddr <= 0) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "No feedback is provided by the relay"));
	}
	std::vector<uint8_t> bits = _endpoint.readInputBits(_feedbackBitAddr, 1);
	return bits[0];
}

} // namespace isl

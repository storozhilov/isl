#include <isl/Relay.hxx>
#include <isl/common.hxx>
#include <isl/LogMessage.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>

namespace isl
{

Relay::Relay(ModbusDevice& device, int bitAddr, int feedbackBitAddr) :
	_device(device),
	_bitAddr(bitAddr),
	_feedbackBitAddr(feedbackBitAddr),
	_state(false),
	_stateRwLock()
{}

bool Relay::state() const
{
	ReadLocker locker(_stateRwLock);
	return _state;
}

void Relay::setState(bool newValue)
{
	_device.writeBit(_bitAddr, newValue);
	WriteLocker locker(_stateRwLock);
	_state = newValue;
}

bool Relay::feedbackState()
{
	if (_feedbackBitAddr <= 0) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Feedback bit address has not been set in relay"));
	}
	std::vector<uint8_t> bits = _device.readBits(_feedbackBitAddr, 1);
	return bits[0];
}

} // namespace isl

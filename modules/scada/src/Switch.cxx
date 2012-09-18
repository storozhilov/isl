#include <isl/Switch.hxx>

namespace isl
{

Switch::Switch(ModbusDevice& device, int stateBitAddr) :
	_device(device),
	_stateBitAddr(stateBitAddr)
{}

bool Switch::state() const
{
	std::vector<uint8_t> bits = _device.readBits(_stateBitAddr, 1);
	return bits[0];
}

} // namespace isl

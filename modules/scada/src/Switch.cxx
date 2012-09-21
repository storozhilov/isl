#include <isl/Switch.hxx>

namespace isl
{

Switch::Switch(ModbusEndpoint& endpoint, int stateBitAddr) :
	_endpoint(endpoint),
	_stateBitAddr(stateBitAddr)
{}

bool Switch::state() const
{
	std::vector<uint8_t> bits = _endpoint.readBits(_stateBitAddr, 1);
	return bits[0];
}

} // namespace isl

#ifndef ISL__SWITCH__HXX
#define ISL__SWITCH__HXX

#include <isl/ModbusDevice.hxx>

namespace isl
{

//! Switch class from the ISL's SCADA module.
class Switch
{
public:
	//! Constructs a relay
	/*!
	  \param device Reference to the modbus device
	  \param stateBitAddr Switch state bit address 
	*/
	Switch(ModbusDevice& device, int stateBitAddr);
	//! Returns switch state bit address
	inline int stateBitAddr() const
	{
		return _stateBitAddr;
	}
	//! Returns current switch state
	/*!
	  Sends "read input status" (0x02) modbus command to the device and returns the data from the response

	  \return Switch register value
	*/
	bool state() const;
private:
	Switch();
	Switch(const Switch&);								// No copy

	Switch& operator=(const Switch&);						// No copy

	ModbusDevice& _device;
	int _stateBitAddr;
};

} // namespace isl

#endif

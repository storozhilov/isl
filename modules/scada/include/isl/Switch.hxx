#ifndef ISL__SWITCH__HXX
#define ISL__SWITCH__HXX

#include <isl/ModbusEndpoint.hxx>

namespace isl
{

//! Switch class from the ISL's SCADA module.
/*!
  This is a boolean Sensor where it's value is represented by bit not word.
*/
class Switch
{
public:
	//! Constructs a relay
	/*!
	  \param endpoint Reference to the modbus endpoint
	  \param stateBitAddr Switch state bit address 
	*/
	Switch(ModbusEndpoint& endpoint, int stateBitAddr);
	//! Returns switch state bit address
	inline int stateBitAddr() const
	{
		return _stateBitAddr;
	}
	//! Returns current switch state
	/*!
	  Sends "read input status" (0x02) modbus command to the endpoint and returns the data from the response

	  \return Switch register value
	*/
	bool state() const;
private:
	Switch();
	Switch(const Switch&);								// No copy

	Switch& operator=(const Switch&);						// No copy

	ModbusEndpoint& _endpoint;
	int _stateBitAddr;
};

} // namespace isl

#endif

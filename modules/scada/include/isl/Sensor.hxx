#ifndef ISL__SENSOR__HXX
#define ISL__SENSOR__HXX

#include <isl/ModbusEndpoint.hxx>

namespace isl
{

//! Sensor class from the ISL's SCADA module.
class Sensor
{
public:
	//! Constructs a relay
	/*!
	  \param endpoint Reference to the modbus endpoint
	  \param valueRegisterAddr Sensor register address 
	*/
	Sensor(ModbusEndpoint& endpoint, int valueRegisterAddr);
	//! Returns sensor register address
	inline int valueRegisterAddr() const
	{
		return _valueRegisterAddr;
	}
	//! Returns current value
	/*!
	  Sends "read input registers" (0x04) modbus command to the endpoint and returns the data from the respons

	  \return Sensor register value
	*/
	uint16_t value() const;
private:
	Sensor();
	Sensor(const Sensor&);								// No copy

	Sensor& operator=(const Sensor&);						// No copy

	ModbusEndpoint& _endpoint;
	int _valueRegisterAddr;
};

} // namespace isl

#endif

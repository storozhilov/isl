#ifndef ISL__CONTROL__HXX
#define ISL__CONTROL__HXX

#include <isl/ModbusEndpoint.hxx>

namespace isl
{

//! Control class from the ISL's SCADA module.
class Control
{
public:
	//! Constructs a relay
	/*!
	  \param endpoint Reference to the modbus endpoint
	  \param valueRegisterAddr Control's value register address 
	  \param feedbackRegisterAddr Feedback value input register address or 0 if no feedback is provided by the control
	*/
	Control(ModbusEndpoint& endpoint, int valueRegisterAddr, int feedbackRegisterAddr = 0);
	//! Returns sensor register address
	inline int valueRegisterAddr() const
	{
		return _valueRegisterAddr;
	}
	//! Returns sensor feedback register address
	inline int feedbackRegisterAddr() const
	{
		return _feedbackRegisterAddr;
	}
	//! Returns current control's value
	/*!
	  Sends "read holding registers" (0x03) modbus command to the endpoint and returns the data from the response

	  \return Control's value
	*/
	uint16_t value() const;
	//! Sets control's value
	/*!
	  Sends "preset single register" (0x06) modbus command to the endpoint

	  \param newValue New control's value
	*/
	void setValue(uint16_t newValue);
	//! Returns control's feedback value
	/*!
	  Sends "read input registers" (0x04) modbus command to the endpoint and returns the data from the response

	  \return Control's feedback value
	*/
	uint16_t feedbackValue() const;
private:
	Control();
	Control(const Control&);							// No copy

	Control& operator=(const Control&);						// No copy

	ModbusEndpoint& _endpoint;
	int _valueRegisterAddr;
	int _feedbackRegisterAddr;
};

} // namespace isl

#endif

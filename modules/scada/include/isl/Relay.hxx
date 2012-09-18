#ifndef ISL__RELAY__HXX
#define ISL__RELAY__HXX

#include <isl/ModbusDevice.hxx>

namespace isl
{

//! Relay class from the ISL's SCADA module.
class Relay
{
public:
	//! Constructs a relay
	/*!
	  \param device Reference to the modbus device
	  \param stateBitAddr State bit address
	  \param feedbackBitAddr Feedback bit address or 0 if no feedback is provided by the relay
	*/
	Relay(ModbusDevice& device, int stateBitAddr, int feedbackBitAddr = 0);
	//! Returns state bit address
	inline int stateBitAddr() const
	{
		return _stateBitAddr;
	}
	//! Returns feedback bit address or 0 if no feedback is provided by the relay
	inline int feedbackBitAddr() const
	{
		return _feedbackBitAddr;
	}
	//! Returns relay state
	/*!
	  Sends "read coil status" (0x01) modbus command to the device and returns the data from the response

	  \return Relay state
	*/
	bool state() const;
	//! Sets the new relay state
	/*!
	  Sends "preset single register" (0x06) modbus command to the device.

	  \param newValue New relay state
	*/
	void setState(bool newValue);
	//! Returns a feedback bit state
	/*!
	  Sends "read input status" (0x02) modbus command to the device and returns the data from the response

	  \return Relay relay feedback state
	*/
	bool feedbackState();
private:
	Relay();
	Relay(const Relay&);								// No copy

	Relay& operator=(const Relay&);							// No copy

	ModbusDevice& _device;
	int _stateBitAddr;
	int _feedbackBitAddr;
};

} // namespace isl

#endif

#ifndef ISL__RELAY__HXX
#define ISL__RELAY__HXX

#include <isl/ModbusEndpoint.hxx>

namespace isl
{

//! Relay class from the ISL's SCADA module.
class Relay
{
public:
	//! Constructs a relay
	/*!
	  \param endpoint Reference to the modbus endpoint
	  \param stateBitAddr State bit address
	  \param feedbackBitAddr Feedback bit address or 0 if no feedback is provided by the relay
	*/
	Relay(ModbusEndpoint& endpoint, int stateBitAddr, int feedbackBitAddr = 0);
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
	//! Inspects if the relay has a feedback channel
	inline bool hasFeedback() const
	{
		return _feedbackBitAddr > 0;
	}
	//! Returns relay state
	/*!
	  Sends "read coil status" (0x01) modbus command to the endpoint and returns the data from the response

	  \return Relay state
	*/
	bool state() const;
	//! Sets the new relay state
	/*!
	  Sends "preset single register" (0x06) modbus command to the endpoint.

	  \param newValue New relay state
	*/
	void setState(bool newValue);
	//! Returns a feedback bit state
	/*!
	  Sends "read input status" (0x02) modbus command to the endpoint and returns the data from the response

	  \return Relay relay feedback state
	*/
	bool feedbackState();
private:
	Relay();
	Relay(const Relay&);								// No copy

	Relay& operator=(const Relay&);							// No copy

	ModbusEndpoint& _endpoint;
	int _stateBitAddr;
	int _feedbackBitAddr;
};

} // namespace isl

#endif

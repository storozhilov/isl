#ifndef ISL__VALVE__HXX
#define ISL__VALVE__HXX

#include <isl/ModbusEndpoint.hxx>
#include <isl/Timeout.hxx>

namespace isl
{

//! Valve class from the ISL's SCADA module.
/*!
  Valve has bidirectional motor to open/close the door and two limit switches which indicate
  that power supply to the motor while opening/closing the door should be interrupted.
*/
class Valve
{
public:
	//! Door drive power off reason
	enum PowerOffReason {
		UndefinedReason = 0,			//!< Undefined reason
		LimitSwitchReason = 1,			//!< Power off caused by limit switch triggering
		TimeoutExpiredReason = 2,		//!< Power off caused by power supply duration timeout expired
		StopCommandReason = 3,			//!< Power off caused by resetting a power supply bit MODBUS-command
		ReverseCommandReason = 4		//!< Power off caused by setting a reverse power supply bit MODBUS-command
	};
	//! Constructs a valve
	/*!
	  \param endpoint Reference to the modbus endpoint
	  \param openDriveStateBitAddr Opening door drive power supply state bit address
	  \param closeDriveStateBitAddr Closing door drive power supply state bit address
	  \param openLimitSwitchStateBitAddr Opening door limit switch state input bit address
	  \param closeLimitSwitchStateBitAddr Closing door limit switch state input bit address
	  \param maxOpenDurationRegisterAddr Maximum opening door power supply duration register address (if the limit switch had not beed switched)
	  \param maxCloseDurationRegisterAddr Maximum closing door power supply duration register address (if the limit switch had not beed switched)
	  \param openPowerOffReasonRegisterAddr Last open drive power off reason input register address
	  \param closePowerOffReasonRegisterAddr Last close drive power off reason input register address
	*/
	Valve(ModbusEndpoint& endpoint, int openDriveStateBitAddr, int closeDriveStateBitAddr, int openLimitSwitchStateBitAddr,
			int closeLimitSwitchStateBitAddr, int maxOpenDurationRegisterAddr, int maxCloseDurationRegisterAddr,
			int openPowerOffReasonRegisterAddr, int closePowerOffReasonRegisterAddr);
	//! Returns open drive state
	/*!
	  Sends "read coil status" (0x01) modbus command to the endpoint and returns the data from the response

	  \return Open drive state
	*/
	bool openDriveState() const;
	//! Sets open drive state
	/*!
	  Sends "force single coil" (0x05) modbus command to the endpoint.

	  \param newValue New open drive state
	*/
	void setOpenDriveState(bool newValue);
	//! Returns close drive state
	/*!
	  Sends "read coil status" (0x01) modbus command to the endpoint and returns the data from the response

	  \return Close drive state
	*/
	bool closeDriveState() const;
	//! Sets close drive state
	/*!
	  Sends "force single coil" (0x05) modbus command to the endpoint.

	  \param newValue New close drive state
	*/
	void setCloseDriveState(bool newValue);
	//! Returns an open limit switch state
	/*!
	  Sends "read input status" (0x02) modbus command to the endpoint and returns the data from the response

	  \return Relay relay feedback state
	*/
	bool openLimitSwitchState() const;
	//! Returns a close limit switch state
	/*!
	  Sends "read input status" (0x02) modbus command to the endpoint and returns the data from the response

	  \return Relay relay feedback state
	*/
	bool closeLimitSwitchState() const;
	//! Returns maximum opening door power supply duration
	/*!
	  Sends "read holding registers" (0x03) modbus command to the endpoint and returns the data from the response

	  \return Maximum opening door power supply duration
	*/
	Timeout maxOpenDuration() const;
	//! Sets maximum opening door power supply duration
	/*!
	  Sends "preset single register" (0x06) modbus command to the endpoint

	  \return Maximum opening door power supply duration
	*/
	void setMaxOpenDuration(const Timeout& newValue);
	//! Returns maximum closing door power supply duration
	/*!
	  Sends "read holding registers" (0x03) modbus command to the endpoint and returns the data from the response

	  \return Maximum closing door power supply duration
	*/
	Timeout maxCloseDuration() const;
	//! Sets maximum closing door power supply duration
	/*!
	  Sends "preset single register" (0x06) modbus command to the endpoint

	  \return Maximum closing door power supply duration
	*/
	void setMaxCloseDuration(const Timeout& newValue);
	//! Returns last open drive power off reason
	/*!
	  Sends "read input registers" (0x04) modbus command to the endpoint and returns the data from the response

	  \return Last open drive power off reason
	*/
	PowerOffReason openPowerOffReason() const;
	//! Returns last close drive power off reason
	/*!
	  Sends "read input registers" (0x04) modbus command to the endpoint and returns the data from the response

	  \return Last close drive power off reason
	*/
	PowerOffReason closePowerOffReason() const;
private:
	Valve();
	Valve(const Valve&);								// No copy

	Valve& operator=(const Valve&);							// No copy

	ModbusEndpoint& _endpoint;
	int _openDriveStateBitAddr;
	int _closeDriveStateBitAddr;
	int _openLimitSwitchStateBitAddr;
	int _closeLimitSwitchStateBitAddr;
	int _maxOpenDurationRegisterAddr;
	int _maxCloseDurationRegisterAddr;
	int _openPowerOffReasonRegisterAddr;
	int _closePowerOffReasonRegisterAddr;
};

} // namespace isl

#endif

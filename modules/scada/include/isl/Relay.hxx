#ifndef ISL__RELAY__HXX
#define ISL__RELAY__HXX

#include <isl/ModbusDevice.hxx>
#include <isl/ReadWriteLock.hxx>

#ifndef ISL__RELAY_DEFAULT_FEEDBACK_TIMEOUT_SECONDS
#define ISL__RELAY_DEFAULT_FEEDBACK_TIMEOUT_SECONDS 10				// 10 seconds
#endif
#ifndef ISL__RELAY_DEFAULT_FEEDBACK_TIMEOUT_NANO_SECONDS
#define ISL__RELAY_DEFAULT_FEEDBACK_TIMEOUT_NANO_SECONDS 0			// 0 nanoseconds
#endif

namespace isl
{

//! Thread-safe relay
class Relay
{
public:
	//! Constructs a relay
	/*!
	  \param device Reference to the modbus device
	  \param bitAddr State bit address
	  \param feedbackBitAddr Feedback bit address or 0 if no feedback is provided by the relay
	*/
	Relay(ModbusDevice& device, int bitAddr, int feedbackBitAddr = 0);
	//! Returns relay state after last set state operation
	bool state() const;
	//! Sets new state
	/*!
	  \param newValue New relay state
	*/
	void setState(bool newValue);
	//! Fetches and returns a feedback bit state
	bool feedbackState();
private:
	Relay();
	Relay(const Relay&);								// No copy

	Relay& operator=(const Relay&);							// No copy

	ModbusDevice& _device;
	int _bitAddr;
	int _feedbackBitAddr;
	bool _state;
	mutable ReadWriteLock _stateRwLock;
};

} // namespace isl

#endif

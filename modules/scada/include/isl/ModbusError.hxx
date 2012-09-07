#ifndef ISL__MODBUS_ERROR__HXX
#define ISL__MODBUS_ERROR__HXX

#include <isl/AbstractError.hxx>

namespace isl
{

//! Modbus error
/*!
  This class belongs to the SCADA module of the ISL. You should enable appropriate building option (see 'scons -h').
*/
class ModbusError : public AbstractError
{
public:
	//! Constructs modbus error
	/*!
	  You should use SOURCE_LOCATION_ARGS macro as a value for the first parameter

	  \param errnum Errno value
	  \param info User info
	*/
	ModbusError(SOURCE_LOCATION_ARGS_DECLARATION, int errnum, const std::string& info = std::string());
	//! Returns error code
	inline int errnum() const
	{
		return _errnum;
	}
	//! Clones error
	virtual AbstractError * clone() const;
private:
	ModbusError();

	virtual std::string composeMessage() const;

	int _errnum;
};

} // namespace isl

#endif

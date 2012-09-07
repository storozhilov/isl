#include <isl/ModbusError.hxx>
#include <sstream>
#include <modbus.h>

namespace isl
{

ModbusError::ModbusError(SOURCE_LOCATION_ARGS_DECLARATION, int errnum, const std::string& info) :
	AbstractError(SOURCE_LOCATION_ARGS_PASSTHRU, info),
	_errnum(errnum)
{}

AbstractError * ModbusError::clone() const
{
	return new ModbusError(*this);
}

std::string ModbusError::composeMessage() const
{
	std::ostringstream oss;
	oss << "Modbus error: (" << _errnum << ") " << modbus_strerror(_errnum);
	return oss.str();
}

} // namespace isl

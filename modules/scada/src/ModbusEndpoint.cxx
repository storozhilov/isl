#include <isl/ModbusEndpoint.hxx>
#include <isl/ModbusError.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <errno.h>
#include <sstream>

namespace isl
{

ModbusEndpoint::ModbusEndpoint(const std::string& serialDevice, int id, Baud baud, Parity parity, DataBits dataBits, StopBits stopBits) :
	_ctx(),
	_serialDevice(serialDevice),
	_id(id),
	_baud(baud),
	_parity(parity),
	_dataBits(dataBits),
	_stopBits(stopBits)
{
	int baudValue;
	switch (_baud) {
		case Baud_50:
			baudValue = 50;
			break;
		case Baud_75:
			baudValue = 75;
			break;
		case Baud_110:
			baudValue = 110;
			break;
		case Baud_134:
			baudValue = 134;
			break;
		case Baud_150:
			baudValue = 150;
			break;
		case Baud_200:
			baudValue = 200;
			break;
		case Baud_300:
			baudValue = 300;
			break;
		case Baud_600:
			baudValue = 600;
			break;
		case Baud_1200:
			baudValue = 1200;
			break;
		case Baud_1800:
			baudValue = 1800;
			break;
		case Baud_2400:
			baudValue = 2400;
			break;
		case Baud_4800:
			baudValue = 4800;
			break;
		case Baud_9600:
			baudValue = 9600;
			break;
		case Baud_19200:
			baudValue = 19200;
			break;
		case Baud_38400:
			baudValue = 38400;
			break;
		case Baud_57600:
			baudValue = 57600;
			break;
		case Baud_115200:
			baudValue = 115200;
			break;
		case Baud_230400:
			baudValue = 230400;
			break;
		case Baud_460800:
			baudValue = 460800;
			break;
		case Baud_500000:
			baudValue = 500000;
			break;
		case Baud_576000:
			baudValue = 576000;
			break;
		case Baud_921600:
			baudValue = 921600;
			break;
		case Baud_1000000:
			baudValue = 1000000;
			break;
		case Baud_1152000:
			baudValue = 1152000;
			break;
		case Baud_1500000:
			baudValue = 1500000;
			break;
		case Baud_2000000:
			baudValue = 2000000;
			break;
		case Baud_2500000:
			baudValue = 2500000;
			break;
		case Baud_3000000:
			baudValue = 3000000;
			break;
		case Baud_3500000:
			baudValue = 3500000;
			break;
		case Baud_4000000:
			baudValue = 4000000;
			break;
		default:
			std::ostringstream errMsg;
			errMsg << "Invalid baud rate code value: " << _baud;
			throw Exception(Error(SOURCE_LOCATION_ARGS, errMsg.str()));
	}
	char parityValue;
	switch (_parity) {
		case NoneParity:
			parityValue = 'N';
			break;
		case EvenParity:
			parityValue = 'E';
			break;
		case OddParity:
			parityValue = 'O';
			break;
		default:
			std::ostringstream errMsg;
			errMsg << "Invalid parity code value: " << _parity;
			throw Exception(Error(SOURCE_LOCATION_ARGS, errMsg.str()));
	}
	int dataBitsValue;
	switch (_dataBits) {
		case FiveDataBits:
			dataBitsValue = 5;
			break;
		case SixDataBits:
			dataBitsValue = 6;
			break;
		case SevenDataBits:
			dataBitsValue = 7;
			break;
		case EightDataBits:
			dataBitsValue = 8;
			break;
		default:
			std::ostringstream errMsg;
			errMsg << "Invalid data bits code value: " << _dataBits;
			throw Exception(Error(SOURCE_LOCATION_ARGS, errMsg.str()));
	};
	int stopBitsValue;
	switch (_stopBits) {
		case OneStopBit:
			stopBitsValue = 1;
			break;
		case TwoStopBits:
			stopBitsValue = 2;
			break;
		default:
			std::ostringstream errMsg;
			errMsg << "Invalid stop bits code value: " << _stopBits;
			throw Exception(Error(SOURCE_LOCATION_ARGS, errMsg.str()));
	};
	_ctx = modbus_new_rtu(_serialDevice.c_str(), baudValue, parityValue, dataBitsValue, stopBitsValue);
	if (!_ctx) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error Modbus-RTU context creation"));
	}
	if (modbus_set_slave(_ctx, _id) < 0) {
		modbus_free(_ctx);
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error setting endpoint ID"));
	}
}

ModbusEndpoint::~ModbusEndpoint()
{
	modbus_free(_ctx);
}

void ModbusEndpoint::open()
{
	if (modbus_connect(_ctx) < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error opening modbus connection"));
	}
}

void ModbusEndpoint::flush()
{
	if (modbus_flush(_ctx) < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error flushing non-transmitted modbus data"));
	}
}

std::vector<uint8_t> ModbusEndpoint::readBits(int addr, int bitsAmount)
{
	std::vector<uint8_t> bits(bitsAmount);
	int bitsFetched = modbus_read_bits(_ctx, addr, bitsAmount, &bits[0]);
	if (bitsFetched < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error reading bits"));
	}
	bits.resize(bitsFetched);
	return bits;
}

std::vector<uint8_t> ModbusEndpoint::readInputBits(int addr, int bitsAmount)
{
	std::vector<uint8_t> bits(bitsAmount);
	int bitsFetched = modbus_read_input_bits(_ctx, addr, bitsAmount, &bits[0]);
	if (bitsFetched < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error reading input bits"));
	}
	bits.resize(bitsFetched);
	return bits;
}

std::vector<uint16_t> ModbusEndpoint::readRegisters(int addr, int registersAmount)
{
	std::vector<uint16_t> registers(registersAmount);
	int registersFetched = modbus_read_registers(_ctx, addr, registersAmount, &registers[0]);
	if (registersFetched < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error reading registers"));
	}
	registers.resize(registersFetched);
	return registers;
}

std::vector<uint16_t> ModbusEndpoint::readInputRegisters(int addr, int registersAmount)
{
	std::vector<uint16_t> registers(registersAmount);
	int registersFetched = modbus_read_input_registers(_ctx, addr, registersAmount, &registers[0]);
	if (registersFetched < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error reading input registers"));
	}
	registers.resize(registersFetched);
	return registers;
}

void ModbusEndpoint::writeBit(int addr, bool value)
{
	if (modbus_write_bit(_ctx, addr, value ? TRUE : FALSE) < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error writing bit"));
	}
}

void ModbusEndpoint::writeRegister(int addr, uint16_t value)
{
	if (modbus_write_register(_ctx, addr, value) < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error writing register"));
	}
}

int ModbusEndpoint::writeBits(int addr, const std::vector<uint8_t>& bits)
{
	int bitsWritten = modbus_write_bits(_ctx, addr, bits.size(), &bits[0]);
	if (bitsWritten < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error writing bits"));
	}
	return bitsWritten;
}

int ModbusEndpoint::writeRegisters(int addr, const std::vector<uint16_t>& registers)
{
	int registersWritten = modbus_write_registers(_ctx, addr, registers.size(), &registers[0]);
	if (registersWritten < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error writing registers"));
	}
	return registersWritten;
}

std::vector<uint16_t> ModbusEndpoint::writeAndReadRegisters(int writeAddr, const std::vector<uint16_t>& writeRegisters, int readAddr, int readRegistersAmount)
{
	std::vector<uint16_t> readRegisters(readRegistersAmount);
	int registersFetched = modbus_write_and_read_registers(_ctx, writeAddr, writeRegisters.size(), &writeRegisters[0], readAddr, readRegistersAmount, &readRegisters[0]);
	if (registersFetched < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error writing and reading registers"));
	}
	readRegisters.resize(registersFetched);
	return readRegisters;
}

} // namespace isl

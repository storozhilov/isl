#include <isl/ModbusEndpoint.hxx>
#include <isl/ModbusError.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <errno.h>
#include <sstream>

namespace isl
{

ModbusEndpoint::ModbusEndpoint(const std::string& serialDevice, int id, Baud baud, Parity parity, DataBits dataBits, StopBits stopBits,
                        const Timeout& idleTimeout) :
	_ctx(),
	_serialDevice(serialDevice),
	_id(id),
	_baudValue(baudToValue(baud)),
	_parityValue(parityToValue(parity)),
	_dataBitsValue(dataBitsToValue(dataBits)),
	_stopBitsValue(stopBitsToValue(stopBits)),
        _idleTimeout(idleTimeout),
        _idleTimeoutCond(),
        _nextOperationLimit(Timestamp::now())
{
	init();
}

ModbusEndpoint::ModbusEndpoint(const std::string& serialDevice, int id, int baudValue, char parityValue, int dataBitsValue, int stopBitsValue,
                        const Timeout& idleTimeout) :
	_ctx(),
	_serialDevice(serialDevice),
	_id(id),
	_baudValue(baudValue),
	_parityValue(parityValue),
	_dataBitsValue(dataBitsValue),
	_stopBitsValue(stopBitsValue),
        _idleTimeout(idleTimeout),
        _idleTimeoutCond(),
        _nextOperationLimit(Timestamp::now())
{
	baudFromValue(baudValue);
	parityFromValue(parityValue);
	dataBitsFromValue(dataBitsValue);
	stopBitsFromValue(stopBitsValue);
	init();
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
        int bitsFetched = 0;
        {
                Locker locker(*this);
                bitsFetched = modbus_read_bits(_ctx, addr, bitsAmount, &bits[0]);
        }
	if (bitsFetched < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error reading bits"));
	}
	bits.resize(bitsFetched);
	return bits;
}

std::vector<uint8_t> ModbusEndpoint::readInputBits(int addr, int bitsAmount)
{
	std::vector<uint8_t> bits(bitsAmount);
	int bitsFetched = 0;
        {
                Locker locker(*this);
                bitsFetched = modbus_read_input_bits(_ctx, addr, bitsAmount, &bits[0]);
        }
	if (bitsFetched < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error reading input bits"));
	}
	bits.resize(bitsFetched);
	return bits;
}

std::vector<uint16_t> ModbusEndpoint::readRegisters(int addr, int registersAmount)
{
	std::vector<uint16_t> registers(registersAmount);
	int registersFetched = 0;
        {
                Locker locker(*this);
                registersFetched = modbus_read_registers(_ctx, addr, registersAmount, &registers[0]);
        }
	if (registersFetched < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error reading registers"));
	}
	registers.resize(registersFetched);
	return registers;
}

std::vector<uint16_t> ModbusEndpoint::readInputRegisters(int addr, int registersAmount)
{
	std::vector<uint16_t> registers(registersAmount);
	int registersFetched = 0;
        {
                Locker locker(*this);
                registersFetched = modbus_read_input_registers(_ctx, addr, registersAmount, &registers[0]);
        }
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
        int res = 0;
        {
                Locker locker(*this);
                res = modbus_write_register(_ctx, addr, value);
        }
	if (res < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error writing register"));
	}
}

int ModbusEndpoint::writeBits(int addr, const std::vector<uint8_t>& bits)
{
	int bitsWritten = 0;
        {
                Locker locker(*this);
                bitsWritten = modbus_write_bits(_ctx, addr, bits.size(), &bits[0]);
        }
	if (bitsWritten < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error writing bits"));
	}
	return bitsWritten;
}

int ModbusEndpoint::writeRegisters(int addr, const std::vector<uint16_t>& registers)
{
	int registersWritten = 0;
        {
                Locker locker(*this);
                registersWritten = modbus_write_registers(_ctx, addr, registers.size(), &registers[0]);
        }
	if (registersWritten < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error writing registers"));
	}
	return registersWritten;
}

std::vector<uint16_t> ModbusEndpoint::writeAndReadRegisters(int writeAddr, const std::vector<uint16_t>& writeRegisters, int readAddr, int readRegistersAmount)
{
	std::vector<uint16_t> readRegisters(readRegistersAmount);
	int registersFetched = 0;
        {
                Locker locker(*this);
                registersFetched = modbus_write_and_read_registers(_ctx, writeAddr, writeRegisters.size(), &writeRegisters[0], readAddr, readRegistersAmount, &readRegisters[0]);
        }
	if (registersFetched < 0) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error writing and reading registers"));
	}
	readRegisters.resize(registersFetched);
	return readRegisters;
}

void ModbusEndpoint::init()
{
	_ctx = modbus_new_rtu(_serialDevice.c_str(), _baudValue, _parityValue, _dataBitsValue, _stopBitsValue);
	if (!_ctx) {
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error Modbus-RTU context creation"));
	}
	if (modbus_set_slave(_ctx, _id) < 0) {
		modbus_free(_ctx);
		throw Exception(ModbusError(SOURCE_LOCATION_ARGS, errno, "Error setting endpoint ID"));
	}
}

int ModbusEndpoint::baudToValue(Baud baud)
{
	switch (baud) {
		case Baud_50:
			return 50;
		case Baud_75:
			return 75;
		case Baud_110:
			return 110;
		case Baud_134:
			return 134;
		case Baud_150:
			return 150;
		case Baud_200:
			return 200;
		case Baud_300:
			return 300;
		case Baud_600:
			return 600;
		case Baud_1200:
			return 1200;
		case Baud_1800:
			return 1800;
		case Baud_2400:
			return 2400;
		case Baud_4800:
			return 4800;
		case Baud_9600:
			return 9600;
		case Baud_19200:
			return 19200;
		case Baud_38400:
			return 38400;
		case Baud_57600:
			return 57600;
		case Baud_115200:
			return 115200;
		case Baud_230400:
			return 230400;
		case Baud_460800:
			return 460800;
		case Baud_500000:
			return 500000;
		case Baud_576000:
			return 576000;
		case Baud_921600:
			return 921600;
		case Baud_1000000:
			return 1000000;
		case Baud_1152000:
			return 1152000;
		case Baud_1500000:
			return 1500000;
		case Baud_2000000:
			return 2000000;
		case Baud_2500000:
			return 2500000;
		case Baud_3000000:
			return 3000000;
		case Baud_3500000:
			return 3500000;
		case Baud_4000000:
			return 4000000;
		default:
			std::ostringstream errMsg;
			errMsg << "Invalid baud rate code value: " << baud;
			throw Exception(Error(SOURCE_LOCATION_ARGS, errMsg.str()));
	}
}

ModbusEndpoint::Baud ModbusEndpoint::baudFromValue(int baudValue)
{
	switch (baudValue) {
		case 50:
			return Baud_50;
		case 75:
			return Baud_75;
		case 110:
			return Baud_110;
		case 134:
			return Baud_134;
		case 150:
			return Baud_150;
		case 200:
			return Baud_200;
		case 300:
			return Baud_300;
		case 600:
			return Baud_600;
		case 1200:
			return Baud_1200;
		case 1800:
			return Baud_1800;
		case 2400:
			return Baud_2400;
		case 4800:
			return Baud_4800;
		case 9600:
			return Baud_9600;
		case 19200:
			return Baud_19200;
		case 38400:
			return Baud_38400;
		case 57600:
			return Baud_57600;
		case 115200:
			return Baud_115200;
		case 230400:
			return Baud_230400;
		case 460800:
			return Baud_460800;
		case 500000:
			return Baud_500000;
		case 576000:
			return Baud_576000;
		case 921600:
			return Baud_921600;
		case 1000000:
			return Baud_1000000;
		case 1152000:
			return Baud_1152000;
		case 1500000:
			return Baud_1500000;
		case 2000000:
			return Baud_2000000;
		case 2500000:
			return Baud_2500000;
		case 3000000:
			return Baud_3000000;
		case 3500000:
			return Baud_3500000;
		case 4000000:
			return Baud_4000000;
		default:
			std::ostringstream errMsg;
			errMsg << "Invalid baud rate value: " << baudValue;
			throw Exception(Error(SOURCE_LOCATION_ARGS, errMsg.str()));
	}
}

char ModbusEndpoint::parityToValue(Parity parity)
{
	switch (parity) {
		case NoneParity:
			return 'N';
		case EvenParity:
			return 'E';
		case OddParity:
			return 'O';
		default:
			std::ostringstream errMsg;
			errMsg << "Invalid parity code value: " << parity;
			throw Exception(Error(SOURCE_LOCATION_ARGS, errMsg.str()));
	}
}

ModbusEndpoint::Parity ModbusEndpoint::parityFromValue(char parityValue)
{
	switch (parityValue) {
		case 'N':
			return NoneParity;
		case 'E':
			return EvenParity;
		case 'O':
			return OddParity;
		default:
			std::ostringstream errMsg;
			errMsg << "Invalid parity value: " << parityValue;
			throw Exception(Error(SOURCE_LOCATION_ARGS, errMsg.str()));
	}
}

int ModbusEndpoint::dataBitsToValue(DataBits dataBits)
{
	switch (dataBits) {
		case FiveDataBits:
			return 5;
		case SixDataBits:
			return 6;
		case SevenDataBits:
			return 7;
		case EightDataBits:
			return 8;
		default:
			std::ostringstream errMsg;
			errMsg << "Invalid data bits code value: " << dataBits;
			throw Exception(Error(SOURCE_LOCATION_ARGS, errMsg.str()));
	};
}

ModbusEndpoint::DataBits ModbusEndpoint::dataBitsFromValue(int dataBitsValue)
{
	switch (dataBitsValue) {
		case 5:
			return FiveDataBits;
		case 6:
			return SixDataBits;
		case 7:
			return SevenDataBits;
		case 8:
			return EightDataBits;
		default:
			std::ostringstream errMsg;
			errMsg << "Invalid data bits value: " << dataBitsValue;
			throw Exception(Error(SOURCE_LOCATION_ARGS, errMsg.str()));
	};
}

int ModbusEndpoint::stopBitsToValue(StopBits stopBits)
{
	switch (stopBits) {
		case OneStopBit:
			return 1;
		case TwoStopBits:
			return 2;
		default:
			std::ostringstream errMsg;
			errMsg << "Invalid stop bits code value: " << stopBits;
			throw Exception(Error(SOURCE_LOCATION_ARGS, errMsg.str()));
	};
}

ModbusEndpoint::StopBits ModbusEndpoint::stopBitsFromValue(int stopBitsValue)
{
	switch (stopBitsValue) {
		case 1:
			return OneStopBit;
		case 2:
			return TwoStopBits;
		default:
			std::ostringstream errMsg;
			errMsg << "Invalid stop bits value: " << stopBitsValue;
			throw Exception(Error(SOURCE_LOCATION_ARGS, errMsg.str()));
	};
}

//------------------------------------------------------------------------------

ModbusEndpoint::Locker::Locker(ModbusEndpoint& endpoint) :
        _endpoint(endpoint),
        _mutexLocker(endpoint._idleTimeoutCond.mutex())
{
        while (true) {
                if (Timestamp::now() >= _endpoint._nextOperationLimit) {
                        return;
                }
                _endpoint._idleTimeoutCond.wait(_endpoint._nextOperationLimit);
        }
}

ModbusEndpoint::Locker::~Locker()
{
        _endpoint._nextOperationLimit = Timestamp::now() + _endpoint._idleTimeout;
}

} // namespace isl

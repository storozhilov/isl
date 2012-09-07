#ifndef ISL__MODBUS_DEVICE__HXX
#define ISL__MODBUS_DEVICE__HXX

#include <modbus.h>
#include <string>
#include <vector>

namespace isl
{

//! MODBUS-endpoint
/*!
  Use this class for SCADA-systems implementation. ISL is very suitable for device-controlling applications development due to it's multithread nature.

  This class belongs to the SCADA module of the ISL. You should enable appropriate building option (see 'scons -h').
*/
class ModbusDevice
{
public:
	//! Baud rate
	enum Baud {
		Baud_50,
		Baud_75,
		Baud_110,
		Baud_134,
		Baud_150,
		Baud_200,
		Baud_300,
		Baud_600,
		Baud_1200,
		Baud_1800,
		Baud_2400,
		Baud_4800,
		Baud_9600,
		Baud_19200,
		Baud_38400,
		Baud_57600,
		Baud_115200,
		Baud_230400,
		Baud_460800,
		Baud_500000,
		Baud_576000,
		Baud_921600,
		Baud_1000000,
		Baud_1152000,
		Baud_1500000,
		Baud_2000000,
		Baud_2500000,
		Baud_3000000,
		Baud_3500000,
		Baud_4000000
	};
	//! Parity
	enum Parity {
		NoneParity,
		EvenParity,
		OddParity
	};
	//! Data bits
	enum DataBits {
		FiveDataBits,
		SixDataBits,
		SevenDataBits,
		EightDataBits
	};
	//! Stop bits
	enum StopBits {
		OneStopBit,
		TwoStopBits
	};
	//! Constructs Modbus/RTU endpoint
	/*!
	  \param serialDevice Serial device filename (e.g. /dev/ttyS0 or /dev/ttyUSB0)
	  \param deviceId Id of the Modbus-endpoint
	  \param baud Baud rate
	  \param parity Parity
	  \param dataBits Data bits
	  \param stopBits Stop bits
	*/
	ModbusDevice(const std::string& serialDevice, int deviceId, Baud baud, Parity parity, DataBits dataBits, StopBits stopBits);
	//! Destructor
	~ModbusDevice();
	//! Returns constant reference to the serial device filename
	inline const std::string& serialDevice() const
	{
		return _serialDevice;
	}
	//! Returns Id of the Modbus-endpoint
	inline int deviceId() const
	{
		return _deviceId;
	}
	//! Returns baud rate
	inline Baud baud() const
	{
		return _baud;
	}
	//! Returns parity
	inline Parity parity() const
	{
		return _parity;
	}
	//! Returns databits
	inline DataBits dataBits() const
	{
		return _dataBits;
	}
	//! Returns stop bits
	inline StopBits stopBits() const
	{
		return _stopBits;
	}
	//! Opens a Modbus-endpoint
	void open();
	//! Closes a Modbus-endpoint
	inline void close()
	{
		modbus_close(_ctx);
	}
	//! Flushes all insent data to the Modbus-endpoint
	void flush();
	//! Reproduces "read coil status" (0x01) modbus function call
	/*!
	  \param addr First bit address
	  \param bitsAmount Amount of bits to read
	  \return Vector of bits (each bit is represented as uint8_t)
	*/
	std::vector<uint8_t> readBits(int addr, int bitsAmount);
	//! Reproduces "read input status" (0x02) modbus function call
	/*!
	  \param addr First bit address
	  \param bitsAmount Amount of bits to read
	  \return Vector of bits (each bit is represented as uint8_t)
	*/
	std::vector<uint8_t> readInputBits(int addr, int bitsAmount);
	//! Reproduces "read holding registers" (0x03) modbus function call
	/*!
	  \param addr First register address
	  \param registersAmount Amount of registers to read
	  \return Vector of registers
	*/
	std::vector<uint16_t> readRegisters(int addr, int registersAmount);
	//! Reproduces "read input registers" (0x04) modbus function call
	/*!
	  \param addr First register address
	  \param registersAmount Amount of registers to read
	  \return Vector of registers
	*/
	std::vector<uint16_t> readInputRegisters(int addr, int registersAmount);
	//! Reproduces "force single coil" (0x05) modbus function call
	/*!
	  \param addr Bit address
	  \param value Bit value to store
	*/
	void writeBit(int addr, bool value);
	//! Reproduces "preset single register" (0x06) modbus function call
	/*!
	  \param addr Register address
	  \param value Register value to store
	*/
	void writeRegister(int addr, uint16_t value);
	//! Reproduces "force multiple coils" (0x0F) modbus function call
	/*!
	  \param addr First bit address
	  \param bits Values to store
	*/
	int writeBits(int addr, const std::vector<uint8_t>& bits);
	//! Reproduces "preset multiple registers" (0x10) modbus function call
	/*!
	  \param addr First register address
	  \param registers Values to store
	*/
	int writeRegisters(int addr, const std::vector<uint16_t>& registers);
	//! Reproduces "write/read registers" (0x17) modbus function call
	/*!
	  \param writeAddr First writing register address
	  \param writeRegisters Values to store
	  \param readAddr First readinf register address
	  \param readRegistersAmount Read registers amount
	  \return Vector of read registers
	*/
	std::vector<uint16_t> writeAndReadRegisters(int writeAddr, const std::vector<uint16_t>& writeRegisters, int readAddr, int readRegistersAmount);
private:
	ModbusDevice();

	ModbusDevice(const ModbusDevice&);
	ModbusDevice& operator=(const ModbusDevice&);

	modbus_t * _ctx;
	std::string _serialDevice;
	int _deviceId;
	Baud _baud;
	Parity _parity;
	DataBits _dataBits;
	StopBits _stopBits;
};

} // namespace isl

#endif

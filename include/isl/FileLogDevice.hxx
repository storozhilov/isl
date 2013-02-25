//#ifndef ISL__FILE_LOG_DEVICE__HXX
//#define ISL__FILE_LOG_DEVICE__HXX
//
//#include <isl/AbstractLogDevice.hxx>
//#include <isl/DateTime.hxx>
//#include <string>
//#include <sys/stat.h>
//
//namespace isl
//{
//
//class AbstractLogTarget;
//
////! File logging device
//class FileLogDevice : public AbstractLogDevice
//{
//public:
//	//! Constructor
//	/*!
//	  \param fileName Log file name
//	*/
//	FileLogDevice(const std::string& fileName);
//	virtual ~FileLogDevice();
//private:
//	FileLogDevice();
//	FileLogDevice(const FileLogDevice&);
//
//	FileLogDevice& operator=(const FileLogDevice&);
//	//! Returns true if the log device is serving passed log target
//	/*!
//	  \param target Pointer to the log target to check if the device is serving it or not.
//	*/
//	virtual bool serving(const AbstractLogTarget * target) const;
//	//! Thread unsafely writing log message to the log device abstract virtual method
//	/*!
//	  \param log Constant reference to log object
//	  \param msg Constant reference to log message object to write to the log device
//	*/
//	virtual void writeMessage(const Log& log, const AbstractLogMessage& msg);
//
//	std::string _fileName;
//	int _fileDescriptor;
//	dev_t _fileDeviceID;
//	ino_t _fileINodeNumber;
//};
//
//} // namespace isl
//
//#endif
//

#ifndef ISL__FILE_LOG_DEVICE__HXX
#define ISL__FILE_LOG_DEVICE__HXX

#include <isl/AbstractLogDevice.hxx>
#include <isl/DateTime.hxx>
#include <string>
#include <sys/stat.h>

namespace isl
{

class AbstractLogTarget;

class FileLogDevice : public AbstractLogDevice
{
public:
	FileLogDevice(const std::string& fileName);
	~FileLogDevice();

	virtual bool serving(const AbstractLogTarget * target) const;
private:
	FileLogDevice();
	FileLogDevice(const FileLogDevice&);

	FileLogDevice& operator=(const FileLogDevice&);

	void writeMessage(const std::wstring& prefix, const std::wstring& msg);

	std::string _fileName;
	int _fileDescriptor;
	dev_t _fileDeviceID;
	ino_t _fileINodeNumber;
};

} // namespace isl

#endif


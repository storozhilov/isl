#ifndef ISL__FILE_LOG_DEVICE__HXX
#define ISL__FILE_LOG_DEVICE__HXX

#include <isl/AbstractLogDevice.hxx>
#include <isl/DateTime.hxx>
#include <string>

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
	int _fileDeviceID;
	int _fileINodeNumber;
};

} // namespace isl

#endif


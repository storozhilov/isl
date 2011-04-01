#ifndef ISL__FILE_LOG_DEVICE__HXX
#define ISL__FILE_LOG_DEVICE__HXX

#include <isl/AbstractLogDevice.hxx>
#include <isl/FormattedString.hxx>
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

	std::wstring substitute(wchar_t fmt, const std::wstring& param = std::wstring());

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

	FormattedWString<FileLogDevice> firstLineFormat;
	FormattedWString<FileLogDevice> _firstLinePrefixedFormat;
	FormattedWString<FileLogDevice> _secondLineFormat;
	FormattedWString<FileLogDevice> _secondLinePrefixedFormat;
	DateTime _timestampNew;
	std::wstring _prefixNew;
	std::wstring _currentLineNew;
};

} // namespace isl

#endif


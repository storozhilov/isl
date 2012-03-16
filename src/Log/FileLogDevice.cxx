#include <isl/FileLogDevice.hxx>
#include <isl/FileLogTarget.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

namespace isl
{

/*------------------------------------------------------------------------------
 * FileLogDevice
 *------------------------------------------------------------------------------*/

FileLogDevice::FileLogDevice(const std::string& fileName) :
	AbstractLogDevice(),
	_fileName(fileName),
	_fileDescriptor(0),
	_fileDeviceID(0),
	_fileINodeNumber(0)
{
	_fileDescriptor = open(_fileName.c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (_fileDescriptor < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Open, errno));
	}
	struct stat sb;
	if (fstat(_fileDescriptor, &sb) != 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::FStat, errno));
	}
	_fileDeviceID = sb.st_dev;
	_fileINodeNumber = sb.st_ino;
}

FileLogDevice::~FileLogDevice()
{
	if (close(_fileDescriptor) != 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Close, errno));
	}
}

bool FileLogDevice::serving(const AbstractLogTarget * target) const
{
	const FileLogTarget * fileLogTarget = dynamic_cast<const FileLogTarget *>(target);
	if (!fileLogTarget) {
		return false;
	}
	struct stat sb;
	if (stat(fileLogTarget->fileName().c_str(), &sb) != 0) {
		if (errno == ENOENT) {
			return false;
		} else {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Stat, errno));
		}
	}
	return (_fileDeviceID == sb.st_dev) && (_fileINodeNumber == sb.st_ino);

}

void FileLogDevice::writeMessage(const std::wstring& prefix, const std::wstring& msg)
{
	DateTime now = DateTime::now();
	bool isFirstLine = true;
	size_t curPos = 0;
	do {
		size_t crlfPos = msg.find(L"\r\n", curPos);
		size_t crPos = msg.find(L"\n", curPos);
		size_t endlPos = (crlfPos < crPos) ? crlfPos : crPos;
		std::wstring currentLine = msg.substr(curPos, endlPos - curPos);
		curPos = (endlPos == std::wstring::npos) ? std::wstring::npos : ((crlfPos < crPos) ? endlPos + 2 : endlPos + 1);
		std::string stringToWrite = now.toString("%Y-%m-%d %H:%M:%S.%f");
		if (isFirstLine) {
			stringToWrite += ": ";
		} else {
			stringToWrite += "> ";
		}
		if (!prefix.empty()) {
			stringToWrite += '[';
			stringToWrite += String::utf8Encode(prefix);
			stringToWrite += "] ";
		}
		stringToWrite += String::utf8Encode(currentLine);
		stringToWrite += '\n';
		if (write(_fileDescriptor, stringToWrite.data(), stringToWrite.size()) < 0) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Write, errno));
		}
		isFirstLine = false;
	} while (curPos != std::wstring::npos);
}


} // namespace isl


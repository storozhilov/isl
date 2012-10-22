#include <isl/PidFile.hxx>
#include <isl/common.hxx>
#include <isl/ErrorLogMessage.hxx>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace isl
{

PidFile::PidFile(const char * fileName) :
	_fileName(fileName)
{
	init();
}

PidFile::PidFile(const std::string& fileName) :
	_fileName(fileName)
{
	init();
}

PidFile::~PidFile()
{
	if (unlink(_fileName.c_str())) {
		errorLog().log(ErrorLogMessage(SOURCE_LOCATION_ARGS, SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Unlink, errno)));
	}
}

void PidFile::init()
{
	int fd = open(_fileName.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Open, errno));
	}
	std::ostringstream osstr;
	osstr << getpid();
	if (write(fd, osstr.str().data(), osstr.str().size()) < 0) {
		close(fd);
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Write, errno));
	}
	close(fd);
}

} // namespace isl

#include <isl/Core.hxx>
#include <isl/SystemCallError.hxx>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sstream>

namespace isl
{

Log Core::errorLog;
Log Core::warningLog;
Log Core::debugLog;

void Core::daemonize()
{
	pid_t childPid = ::fork();
	if (childPid < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Fork, childPid));
	}
	if (childPid > 0) {
		::exit(0);
	}
	pid_t newSessionId = ::setsid();
	if (newSessionId < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SetSid, newSessionId));
	}
}

void Core::writePid(const char * pidFileName)
{
	int fd = ::open(pidFileName, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Open, errno));
	}
	std::ostringstream osstr;
	osstr << ::getpid();
	if (::write(fd, osstr.str().data(), osstr.str().size()) < 0) {
		::close(fd);
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Write, errno));
	}
	::close(fd);
}

} // namespace isl


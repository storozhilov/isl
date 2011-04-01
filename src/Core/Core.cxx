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
		throw Exception(SystemCallError(SystemCallError::Fork, childPid, SOURCE_LOCATION_ARGS));
	}
	if (childPid > 0) {
		::exit(0);
	}
	pid_t newSessionId = ::setsid();
	if (newSessionId < 0) {
		throw Exception(SystemCallError(SystemCallError::SetSid, newSessionId, SOURCE_LOCATION_ARGS));
	}
}

void Core::writePid(const char * pidFileName)
{
	int fd = ::open(pidFileName, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0) {
		throw Exception(SystemCallError(SystemCallError::Open, errno, SOURCE_LOCATION_ARGS));
	}
	std::ostringstream osstr;
	osstr << ::getpid();
	if (::write(fd, osstr.str().data(), osstr.str().size()) < 0) {
		::close(fd);
		throw Exception(SystemCallError(SystemCallError::Write, errno, SOURCE_LOCATION_ARGS));
	}
	::close(fd);
}

} // namespace isl


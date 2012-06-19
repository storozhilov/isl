#include <isl/common.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <sstream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

namespace isl
{

#ifndef ISL_ERROR_LOG_PREFIX
#define ISL_ERROR_LOG_PREFIX "ERROR"
#endif

#ifndef ISL_ERROR_LOG_COMPOSE_SOURCE_LOCATION
#define ISL_ERROR_LOG_COMPOSE_SOURCE_LOCATION true
#endif

#ifndef ISL_WARNING_LOG_PREFIX
#define ISL_WARNING_LOG_PREFIX "WARNING"
#endif

#ifndef ISL_WARNING_LOG_COMPOSE_SOURCE_LOCATION
#define ISL_WARNING_LOG_COMPOSE_SOURCE_LOCATION true
#endif

#ifndef ISL_DEBUG_LOG_PREFIX
#define ISL_DEBUG_LOG_PREFIX "DEBUG"
#endif

#ifndef ISL_DEBUG_LOG_COMPOSE_SOURCE_LOCATION
#define ISL_DEBUG_LOG_COMPOSE_SOURCE_LOCATION true
#endif

void daemonize()
{
	pid_t childPid = ::fork();
	if (childPid < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Fork, childPid));
	}
	if (childPid > 0) {
		exit(0);
	}
	pid_t newSessionId = ::setsid();
	if (newSessionId < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SetSid, newSessionId));
	}
}

void writePid(const char * pidFileName)
{
	int fd = open(pidFileName, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
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

Log& errorLog()
{
	static Log log(ISL_ERROR_LOG_PREFIX, ISL_ERROR_LOG_COMPOSE_SOURCE_LOCATION);
	return log;
}

Log& warningLog()
{
	static Log log(ISL_WARNING_LOG_PREFIX, ISL_WARNING_LOG_COMPOSE_SOURCE_LOCATION);
	return log;
}

DebugLog& debugLog()
{
	static DebugLog log(ISL_DEBUG_LOG_PREFIX, ISL_DEBUG_LOG_COMPOSE_SOURCE_LOCATION);
	return log;
}

} // namespace isl

#include <isl/TimeSpec.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/SystemCallError.hxx>
#include <errno.h>

namespace isl
{

struct timespec TimeSpec::makeTimestamp(time_t sec, long int nsec)
{
	struct timespec ts;
	ts.tv_sec = sec + nsec / 1000000000L;
	ts.tv_nsec = nsec % 1000000000L;
	// Negative values are forbidden in linux kernel but not here :)
	if (ts.tv_sec < 0 && ts.tv_nsec > 0) {
		--ts.tv_sec;
		ts.tv_nsec -= 1000000000L;
	} else if (ts.tv_sec > 0 && ts.tv_nsec < 0) {
		++ts.tv_sec;
		ts.tv_nsec += 1000000000L;
	}
	return ts;
}

struct timespec TimeSpec::makeTimeout(time_t sec, long int nsec)
{
	struct timespec ts;
	ts.tv_sec = (sec < 0 || nsec < 0) ? 0 : sec + nsec / 1000000000L;
	ts.tv_nsec = (sec < 0 || nsec < 0) ? 0 : nsec % 1000000000L;
	return ts;
}

struct timespec TimeSpec::now()
{
	timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::ClockGetTime, errno, "Fetching current time error"));
	}
	return ts;
}

bool operator==(const struct timespec& lhs, const struct timespec& rhs)
{
	return lhs.tv_sec == rhs.tv_sec && lhs.tv_nsec == rhs.tv_nsec;
}

bool operator!=(const struct timespec& lhs, const struct timespec& rhs)
{
	return !operator==(lhs, rhs);
}

bool operator<(const struct timespec& lhs, const struct timespec& rhs)
{
	return lhs.tv_sec < rhs.tv_sec || (lhs.tv_sec == rhs.tv_sec && lhs.tv_nsec < rhs.tv_nsec);
}

bool operator<=(const struct timespec& lhs, const struct timespec& rhs)
{
	return operator<(lhs, rhs) || operator==(lhs, rhs);
}

bool operator>(const struct timespec& lhs, const struct timespec& rhs)
{
	return lhs.tv_sec > rhs.tv_sec || (lhs.tv_sec == rhs.tv_sec && lhs.tv_nsec > rhs.tv_nsec);
}

bool operator>=(const struct timespec& lhs, const struct timespec& rhs)
{
	return operator>(lhs, rhs) || operator==(lhs, rhs);
}

} // namespace isl

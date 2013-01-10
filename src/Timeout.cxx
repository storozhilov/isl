#include <isl/Timeout.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>

#ifndef ISL__DEFAULT_TIMEOUT_SECONDS
#define ISL__DEFAULT_TIMEOUT_SECONDS 0
#endif
#ifndef ISL__DEFAULT_TIMEOUT_NANO_SECONDS
#define ISL__DEFAULT_TIMEOUT_NANO_SECONDS 100000000				// 100 milliseconds
#endif

namespace isl
{

Timeout::Timeout(time_t secs, long int nsecs) :
	_ts(TimeSpec::makeTimeout(secs, nsecs))
{}

Timeout::Timeout(const struct timespec& ts) :
	_ts(TimeSpec::makeTimeout(ts))
{}

Timeout Timeout::defaultTimeout()
{
	return Timeout(ISL__DEFAULT_TIMEOUT_SECONDS, ISL__DEFAULT_TIMEOUT_NANO_SECONDS);
}

Timeout operator/(const Timeout& lhs, size_t rhs)
{
	if (rhs == 0) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Timeout division by zero error"));
	}
	return Timeout(TimeSpec::makeTimeout(lhs.timeSpec().tv_sec / rhs, lhs.timeSpec().tv_nsec / rhs));
}

} // namespace isl

#include <isl/Timeout.hxx>

#ifndef ISL__DEFAULT_TIMEOUT_SECONDS
#define ISL__DEFAULT_TIMEOUT_SECONDS 0
#endif
#ifndef ISL__DEFAULT_TIMEOUT_NANO_SECONDS
#define ISL__DEFAULT_TIMEOUT_NANO_SECONDS 100000000				// 100 milliseconds
#endif

namespace isl
{

struct timespec Timeout::limit() const
{
	timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	timespec result;
	result.tv_nsec = (now.tv_nsec + nanoSeconds()) % 1000000000;
	result.tv_sec = now.tv_sec + seconds() + (now.tv_nsec + nanoSeconds()) / 1000000000;
	return result;
}

Timeout Timeout::defaultTimeout()
{
	return Timeout(ISL__DEFAULT_TIMEOUT_SECONDS, ISL__DEFAULT_TIMEOUT_NANO_SECONDS);
}

Timeout Timeout::leftToLimit(const struct timespec& limit)
{
	timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	if (now.tv_sec > limit.tv_sec || (now.tv_sec == limit.tv_sec && now.tv_nsec >= limit.tv_nsec)) {
		// Time has been elapsed
		return Timeout();
	} else {
		return Timeout(
			limit.tv_nsec >= now.tv_nsec ? limit.tv_sec - now.tv_sec : limit.tv_sec - now.tv_sec - 1,
			limit.tv_nsec >= now.tv_nsec ? limit.tv_nsec - now.tv_nsec : limit.tv_nsec + 1000000000 - now.tv_nsec
		);
	}
}

struct timespec Timeout::initialTimeSpec(time_t secs, long int nsecs)
{
	struct timespec ts;
	if (secs < 0 || nsecs < 0) {
		ts.tv_sec = 0;
		ts.tv_nsec = 0;
	} else {
		ts.tv_sec = secs + nsecs / 1000000000L;
		ts.tv_nsec = nsecs % 1000000000L;
	}
	return ts;
}

} // namespace isl

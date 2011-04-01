#ifndef ISL__TIMEOUT__HXX
#define ISL__TIMEOUT__HXX

#include <time.h>

namespace isl
{

class Timeout
{
public:
	Timeout(unsigned int seconds = 0, unsigned int nanoSeconds = 0) :
		_seconds(seconds + nanoSeconds / 1000000000),
		_nanoSeconds(nanoSeconds % 1000000000)
	{}

	unsigned int seconds() const
	{
		return _seconds;
	}
	unsigned int nanoSeconds() const
	{
		return _nanoSeconds;
	}
	timespec limit() const
	{
		timespec startTime;
		clock_gettime(CLOCK_REALTIME, &startTime);
		timespec timeoutLimit;
		timeoutLimit.tv_nsec = startTime.tv_nsec + _nanoSeconds;
		timeoutLimit.tv_sec = startTime.tv_sec + _seconds;
		return timeoutLimit;
	}
private:
	unsigned int _seconds;
	unsigned int _nanoSeconds;
};

} // namespace isl

#endif


#ifndef ISL__TIMEOUT__HXX
#define ISL__TIMEOUT__HXX

#include <time.h>

namespace isl
{

#ifndef ISL__DEFAULT_TIMEOUT_SECONDS
#define ISL__DEFAULT_TIMEOUT_SECONDS 0
#endif
#ifndef ISL__DEFAULT_TIMEOUT_MILLI_SECONDS
#define ISL__DEFAULT_TIMEOUT_MILLI_SECONDS 100
#endif
#ifndef ISL__DEFAULT_TIMEOUT_MICRO_SECONDS
#define ISL__DEFAULT_TIMEOUT_MICRO_SECONDS 0
#endif
#ifndef ISL__DEFAULT_TIMEOUT_NANO_SECONDS
#define ISL__DEFAULT_TIMEOUT_NANO_SECONDS 0
#endif

class Timeout
{
public:
	Timeout(unsigned int seconds = 0, unsigned int milliSeconds = 0, unsigned int microSeconds = 0, unsigned int nanoSeconds = 0) :
		_seconds(seconds + milliSeconds / 1000 + microSeconds / 1000000 + nanoSeconds / 1000000000),
		_nanoSeconds(milliSeconds % 1000 * 1000000 + microSeconds % 1000000 * 1000 + nanoSeconds % 1000000000)
	{}

	unsigned int seconds() const
	{
		return _seconds;
	}
	unsigned int milliSeconds() const
	{
		return _nanoSeconds / 1000000;
	}
	unsigned int microSeconds() const
	{
		return _nanoSeconds % 1000000 / 1000;
	}
	unsigned int nanoSeconds() const
	{
		return _nanoSeconds % 1000;
	}
	timespec timeSpec() const
	{
		timespec result;
		result.tv_sec = _seconds;
		result.tv_nsec = _nanoSeconds;
		return result;
	}
	timespec limit() const
	{
		timespec now;
		clock_gettime(CLOCK_REALTIME, &now);
		timespec result;
		result.tv_nsec = (now.tv_nsec + _nanoSeconds) % 1000000000;
		result.tv_sec = now.tv_sec + _seconds + (now.tv_nsec + _nanoSeconds) / 1000000000;
		return result;
	}
	inline bool isZero() const
	{
		return (_seconds == 0) && (_nanoSeconds == 0);
	}
	inline void reset()
	{
		_seconds = 0;
		_nanoSeconds = 0;
	}

	inline static Timeout defaultTimeout()
	{
		return Timeout(ISL__DEFAULT_TIMEOUT_SECONDS, ISL__DEFAULT_TIMEOUT_MILLI_SECONDS, ISL__DEFAULT_TIMEOUT_MICRO_SECONDS, ISL__DEFAULT_TIMEOUT_NANO_SECONDS);
	}
private:
	unsigned int _seconds;
	unsigned int _nanoSeconds;
};

} // namespace isl

#endif


#ifndef ISL__TIMEOUT__HXX
#define ISL__TIMEOUT__HXX

#include <time.h>

namespace isl
{

#ifndef ISL__DEFAULT_TIMEOUT_SECONDS
#define ISL__DEFAULT_TIMEOUT_SECONDS 0
#endif
#ifndef ISL__DEFAULT_TIMEOUT_NANO_SECONDS
#define ISL__DEFAULT_TIMEOUT_NANO_SECONDS 100000000				// 100 milliseconds
#endif

//! Time interval representation class
/*!
  TODO Addition, subtraction, icreasing and decreasing operators.
  TODO Negative timeout support???
*/
class Timeout
{
public:
	//! Constructs timeout from seconds and nanoseconds
	/*!
	  \param seconds Seconds amount
	  \param nanoSeconds Nanoseconds amount
	*/
	Timeout(unsigned long int seconds = 0, unsigned long int nanoSeconds = 0) :
		_seconds(seconds + nanoSeconds / 1000000000L),
		_nanoSeconds(nanoSeconds % 1000000000L)
	{}
	//! Constructs timeout from the as libc timespec structure
	/*!
	  \param interval Libc timespec structure
	*/
	Timeout(const timespec& interval) :
		_seconds(interval.tv_sec + interval.tv_nsec / 1000000000),
		_nanoSeconds(interval.tv_nsec % 1000000000)
	{}
	//! Returns seconds of the timeout
	inline unsigned int seconds() const
	{
		return _seconds;
	}
	//! Returns nanoseconds of the timeout
	inline unsigned int nanoSeconds() const
	{
		return _nanoSeconds;
	}
	//! Returns libc timespec structure representation of the timeout
	timespec timeSpec() const
	{
		timespec result;
		result.tv_sec = _seconds;
		result.tv_nsec = _nanoSeconds;
		return result;
	}
	//! Returns time limit (current timestamp plus timeout) as libc timespec structure
	timespec limit() const
	{
		timespec now;
		clock_gettime(CLOCK_REALTIME, &now);
		timespec result;
		result.tv_nsec = (now.tv_nsec + _nanoSeconds) % 1000000000;
		result.tv_sec = now.tv_sec + _seconds + (now.tv_nsec + _nanoSeconds) / 1000000000;
		return result;
	}
	//! Returns TRUE if the timeout is holding zero values
	inline bool isZero() const
	{
		return (_seconds == 0) && (_nanoSeconds == 0);
	}
	//! Resets timeout to zero value
	inline void reset()
	{
		_seconds = 0;
		_nanoSeconds = 0;
	}
	//! Returns a default timeout
	inline static Timeout defaultTimeout()
	{
		return Timeout(ISL__DEFAULT_TIMEOUT_SECONDS, ISL__DEFAULT_TIMEOUT_NANO_SECONDS);
	}
	//! Returns a timeout which is left to the time limit
	/*!
	  \param limit Time limit as libc timespec structure
	  \return Timeout left to the time limit
	*/
	static Timeout leftToLimit(const timespec& limit)
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
private:
	unsigned long int _seconds;
	unsigned long int _nanoSeconds;
};

//! Alias for the Timeout class
typedef Timeout Interval;

} // namespace isl

#endif


#ifndef ISL__TIMEOUT__HXX
#define ISL__TIMEOUT__HXX

#include <isl/BasicDateTime.hxx>

namespace isl
{

//! Time interval representation class
/*!
  \sa DateTime
*/
class Timeout : public BasicDateTime
{
public:
	//! Constructs timeout from seconds and nanoseconds
	/*!
	  If either seconds or nanoseconds are negative it will create a zero timeout.
	  If nanoseconds is more than 999999999 it will append an appropriate amount of
	  seconds to the result and normalize nanoseconds.

	  \param secs Seconds amount
	  \param nsecs Nanoseconds amount
	*/
	Timeout(time_t secs = 0, long int nsecs = 0) :
		BasicDateTime(),
		_ts(initialTimeSpec(secs, nsecs))
	{}
	//! Constructs timeout from the POSIX.1b structure
	/*!
	  \param ts Libc timespec structure
	*/
	Timeout(const struct timespec& ts) :
		BasicDateTime(),
		_ts(initialTimeSpec(ts.tv_sec, ts.tv_nsec))
	{}
	//! Returns seconds of the timeout
	inline time_t seconds() const
	{
		return _ts.tv_sec;
	}
	//! Returns nanoseconds of the timeout (0-999999999)
	inline long int nanoSeconds() const
	{
		return _ts.tv_nsec;
	}
	//! Returns POSIX.1b representation of the timeout
	inline const struct timespec& timeSpec() const
	{
		return _ts;
	}
	//! Returns time limit (current timestamp plus timeout) as POSIX.1b structure
	struct timespec limit() const;
	//! Returns TRUE if the timeout is holding zero values
	inline bool isZero() const
	{
		return (seconds() == 0) && (nanoSeconds() == 0);
	}
	//! Resets timeout to zero value
	inline void reset()
	{
		resetTimeSpec(_ts);
	}
	//! Comparison operator
	/*!
	  \param rhs Another timeout to compare with
	*/
	inline bool operator==(const Timeout& rhs) const
	{
		return seconds() == rhs.seconds() && nanoSeconds() == rhs.nanoSeconds();
	}
	//! Comparison operator
	/*!
	  \param rhs Another timeout to compare with
	*/
	inline bool operator!=(const Timeout& rhs) const
	{
		return !operator==(rhs);
	}
	//! Comparison operator
	/*!
	  \param rhs Another timeout to compare with
	*/
	inline bool operator<(const Timeout& rhs) const
	{
		return seconds() < rhs.seconds() || (seconds() == rhs.seconds() && nanoSeconds() < rhs.nanoSeconds());
	}
	//! Comparison operator
	/*!
	  \param rhs Another timeout to compare with
	*/
	inline bool operator<=(const Timeout& rhs) const
	{
		return operator<(rhs) || operator==(rhs);
	}
	//! Comparison operator
	/*!
	  \param rhs Another timeout to compare with
	*/
	inline bool operator>(const Timeout& rhs) const
	{
		return seconds() > rhs.seconds() || (seconds() == rhs.seconds() && nanoSeconds() > rhs.nanoSeconds());
	}
	//! Comparison operator
	/*!
	  \param rhs Another timeout to compare with
	*/
	inline bool operator>=(const Timeout& rhs) const
	{
		return operator>(rhs) || operator==(rhs);
	}
	//! Returns a default ISL timeout
	/*!
	  You can tune up default ISL timeout properties by defining <tt>ISL__DEFAULT_TIMEOUT_SECONDS</tt> and
	  <tt>ISL__DEFAULT_TIMEOUT_NANO_SECONDS</tt> macros during it's build.
	*/
	static Timeout defaultTimeout();
	//! Returns a timeout which is left from now to the time limit
	/*!
	  \param limit Time limit as POSIX.1b structure
	  \return Timeout left to the time limit
	*/
	static Timeout leftToLimit(const struct timespec& limit);
private:
	static struct timespec initialTimeSpec(time_t secs, long int nsecs);

	struct timespec _ts;
};

Timeout operator*(const Timeout& lhs, size_t rhs);

} // namespace isl

#endif

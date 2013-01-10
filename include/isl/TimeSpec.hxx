#ifndef ISL__TIME_SPEC__HXX
#define ISL__TIME_SPEC__HXX

#include <time.h>

namespace isl
{

//! POSIX.1b structure utility class
class TimeSpec
{
public:
	//! Makes a POSIX.1b structure for the timestamp
	/*!
	  TODO Handle negative values!

	  \param sec Seconds
	  \param nsec Nanoseconds
	  \return POSIX.1b structure for the timestamp
	*/
	static struct timespec makeTimestamp(time_t sec = 0, long int nsec = 0);
	//! Makes a POSIX.1b structure for the timestamp
	/*!
	  \param POSIX.1b structure
	  \return POSIX.1b structurea for the timestamp
	*/
	inline static struct timespec makeTimestamp(const struct timespec& ts)
	{
		return makeTimestamp(ts.tv_sec, ts.tv_nsec);
	}
	//! Makes a POSIX.1b structure for the timeout
	/*!
	  \param sec Seconds
	  \param nsec Nanoseconds
	  \return POSIX.1b structure for the timeout
	*/
	static struct timespec makeTimeout(time_t sec = 0, long int nsec = 0);
	//! Makes a POSIX.1b structure for the timeout
	/*!
	  \param POSIX.1b structure
	  \return POSIX.1b structurea for the timeout
	*/
	inline static struct timespec makeTimeout(const struct timespec& ts)
	{
		return makeTimeout(ts.tv_sec, ts.tv_nsec);
	}
	//! Returns POSIX.1b structure with zero values
	inline static struct timespec makeZero()
	{
		struct timespec ts;
		reset(ts);
		return ts;
	}
	//! Returns current timestamp as POSIX.1b structure
	static struct timespec now();
	//! Resets POSIX.1b structure to hold zero values
	/*!
	  \param ts POSIX.1b structure to reset
	*/
	inline static void reset(struct timespec& ts)
	{
		ts.tv_sec = 0;
		ts.tv_nsec = 0;
	}
	//! Inspects if POSIX.1b structure holds zero values
	inline static bool isZero(const struct timespec& ts)
	{
		return ts.tv_sec == 0 && ts.tv_nsec == 0;
	}
};

//! POSIX.1b structure comparison operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return TRUE if comparison is truly
*/
bool operator==(const struct timespec& lhs, const struct timespec& rhs);
//! POSIX.1b structure comparison operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return TRUE if comparison is truly
*/
bool operator!=(const struct timespec& lhs, const struct timespec& rhs);
//! POSIX.1b structure comparison operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return TRUE if comparison is truly
*/
bool operator<(const struct timespec& lhs, const struct timespec& rhs);
//! POSIX.1b structure comparison operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return TRUE if comparison is truly
*/
bool operator<=(const struct timespec& lhs, const struct timespec& rhs);
//! POSIX.1b structure comparison operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return TRUE if comparison is truly
*/
bool operator>(const struct timespec& lhs, const struct timespec& rhs);
//! POSIX.1b structure comparison operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return TRUE if comparison is truly
*/
bool operator>=(const struct timespec& lhs, const struct timespec& rhs);
//! POSIX.1b time structure comparison helper class
/*!
  Use it in <tt>std::map</tt> or <tt>std::multimap</tt> instantiation when
  <tt>struct timespec</tt> value is used as key.
*/
class TimeSpecComp
{
public:
	//! Comparison &quot;less than&quot; operator
	/*!
	  \param lhs Left handed side argument
	  \param rhs Right handed side argument
	  \return TRUE if lhs is less than rhs
	*/
	inline bool operator()(const struct timespec& lhs, const struct timespec& rhs)
	{
		return lhs < rhs;
	}
};

} // namespace isl

#endif

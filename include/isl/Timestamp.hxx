#ifndef ISL__TIME_STAMP__HXX
#define ISL__TIME_STAMP__HXX

#include <isl/TimeSpec.hxx>
#include <isl/Timeout.hxx>
#include <time.h>

namespace isl
{

//! Nanosecond-precision timestamp
/*!
  \sa Timeout
*/
class Timestamp
{
public:
	//! Creates a timestamp
	/*!
	  \param sec Seconds from Epoch
	  \param nsec Nanoseconds
	*/
	Timestamp(time_t sec = 0, long int nsec = 0);
	//! Creates a timestamp
	/*!
	  \param ts POSIX.1b structure
	*/
	Timestamp(const struct timespec& ts);

	//! Returns second of the timestamp
	inline time_t second() const
	{
		return _ts.tv_sec;
	}
	//! Returns nanosecond of the timestamp
	inline long int nanoSecond() const
	{
		return _ts.tv_nsec;
	}
	//! Returns POSIX.1b representation of the timestamp
	inline const struct timespec& timeSpec() const
	{
		return _ts;
	}
	//! Returns TRUE if the timestamp is holding zero values
	inline bool isZero() const
	{
		return TimeSpec::isZero(_ts);
	}
	//! Resets timestamp
	inline void reset()
	{
		TimeSpec::reset(_ts);
	}
	//! Returns timeout left to the timestamp
	Timeout leftTo() const;
	//! Comparison operator
	/*!
	  \param rhs Another timestamp to compare with
	*/
	inline bool operator==(const Timestamp& rhs) const
	{
		return _ts == rhs._ts;
	}
	//! Comparison operator
	/*!
	  \param rhs Another timestamp to compare with
	*/
	inline bool operator!=(const Timestamp& rhs) const
	{
		return _ts != rhs._ts;
	}
	//! Comparison operator
	/*!
	  \param rhs Another timestamp to compare with
	*/
	inline bool operator<(const Timestamp& rhs) const
	{
		return _ts < rhs._ts;
	}
	//! Comparison operator
	/*!
	  \param rhs Another timestamp to compare with
	*/
	inline bool operator<=(const Timestamp& rhs) const
	{
		return _ts <= rhs._ts;
	}
	//! Comparison operator
	/*!
	  \param rhs Another timestamp to compare with
	*/
	inline bool operator>(const Timestamp& rhs) const
	{
		return _ts > rhs._ts;
	}
	//! Comparison operator
	/*!
	  \param rhs Another timestamp to compare with
	*/
	inline bool operator>=(const Timestamp& rhs) const
	{
		return _ts >= rhs._ts;
	}
	//! Returns current timestamp
	inline static Timestamp now()
	{
		return Timestamp(TimeSpec::now());
	}
	//! Calculates a limit timestamp for a timeout
	/*!
	  \param timeout Timeout
	  \return limit timestamp for a timeout
	*/
	static Timestamp limit(const Timeout& timeout);
private:
	struct timespec _ts;
};

//! Addition operator
/*!
  \param lhs Timestamp to add to
  \param rhs Timeout to add
  \return Addition result
*/
inline Timestamp operator+(const Timestamp& lhs, const Timeout& rhs)
{
	return Timestamp(TimeSpec::makeTimestamp(lhs.timeSpec().tv_sec + rhs.timeSpec().tv_sec,
				lhs.timeSpec().tv_nsec + rhs.timeSpec().tv_nsec));
}
//! Increment operator
/*!
  \param lhs Timestamp to add to
  \param rhs Timeout to add
  \return Reference to increment result
*/
inline Timestamp& operator+=(Timestamp& lhs, const Timeout& rhs)
{
	lhs = lhs + rhs;
	return lhs;
}
//! Subtration operator
/*!
  \param lhs Timestamp to subtract from
  \param rhs Timeout to subtract
  \return Subtraction result timestamp
*/
inline Timestamp operator-(const Timestamp& lhs, const Timeout& rhs)
{
	return Timestamp(TimeSpec::makeTimestamp(lhs.timeSpec().tv_sec - rhs.timeSpec().tv_sec,
				lhs.timeSpec().tv_nsec - rhs.timeSpec().tv_nsec));
}
//! Subtration operator
/*!
  \param lhs Timestamp to subtract from
  \param rhs Timestamp to subtract
  \return Subtraction result timeout
*/
inline Timeout operator-(const Timestamp& lhs, const Timestamp& rhs)
{
	return Timeout(lhs.timeSpec().tv_sec - rhs.timeSpec().tv_sec, lhs.timeSpec().tv_nsec - rhs.timeSpec().tv_nsec);
}
//! Decrement operator
/*!
  \param lhs Timestamp to subtract from
  \param rhs Timeout to subtract
  \return Reference to decrement result
*/
inline Timestamp& operator-=(Timestamp& lhs, const Timeout& rhs)
{
	lhs = lhs - rhs;
	return lhs;
}

} // namespace isl

#endif

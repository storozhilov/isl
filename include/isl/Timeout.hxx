#ifndef ISL__TIMEOUT__HXX
#define ISL__TIMEOUT__HXX

#include <isl/TimeSpec.hxx>

namespace isl
{

//! Nanosecond-precision time interval
/*!
  \sa Timestamp
*/
class Timeout
{
public:
	//! Constructs timeout from double seconds TODO
	/*!
	  \param secs Seconds amount
	*/
	//Timeout(double secs);
	//! Constructs timeout from seconds and nanoseconds
	/*!
	  If either seconds or nanoseconds are negative it will create a zero timeout.
	  If nanoseconds is more than 999999999 it will append an appropriate amount of
	  seconds to the result and normalize nanoseconds.

	  \param secs Seconds amount
	  \param nsecs Nanoseconds amount
	*/
	Timeout(time_t secs = 0, long int nsecs = 0);
	//! Constructs timeout from the POSIX.1b structure
	/*!
	  \param ts Libc timespec structure
	*/
	Timeout(const struct timespec& ts);
	//! Returns seconds part of the timeout
	inline time_t seconds() const
	{
		return _ts.tv_sec;
	}
	//! Returns nanoseconds part of the timeout (0-999999999)
	inline long int nanoSeconds() const
	{
		return _ts.tv_nsec;
	}
        //! Returns timeout as double seconds value
        inline double secondsDouble() const
        {
                return static_cast<double>(_ts.tv_sec) + static_cast<double>(_ts.tv_nsec) / 1000000000.0;
        }
	//! Returns POSIX.1b representation of the timeout
	inline const struct timespec& timeSpec() const
	{
		return _ts;
	}
	//! Returns TRUE if the timeout is holding zero values
	inline bool isZero() const
	{
		return (seconds() == 0) && (nanoSeconds() == 0);
	}
	//! Resets timeout to zero value
	inline void reset()
	{
		TimeSpec::reset(_ts);
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
private:
	struct timespec _ts;
};

//! Addition operator
/*!
  \param lhs Timeout to add to
  \param rhs Timeout to add
  \return Addition result
*/
inline Timeout operator+(const Timeout& lhs, const Timeout& rhs)
{
	return Timeout(lhs.timeSpec().tv_sec + rhs.timeSpec().tv_sec, lhs.timeSpec().tv_nsec + rhs.timeSpec().tv_nsec);
}
//! Increment operator
/*!
  \param lhs Timeout to add to
  \param rhs Timeout to add
  \return Reference to increment result
*/
inline Timeout& operator+=(Timeout& lhs, const Timeout& rhs)
{
	lhs = lhs + rhs;
	return lhs;
}
//! Subtration operator
/*!
  \param lhs Timeout to subtract from
  \param rhs Timeout to subtract
  \return Subtraction result
*/
inline Timeout operator-(const Timeout& lhs, const Timeout& rhs)
{
	return Timeout(lhs.timeSpec().tv_sec - rhs.timeSpec().tv_sec, lhs.timeSpec().tv_nsec - rhs.timeSpec().tv_nsec);
}
//! Decrement operator
/*!
  \param lhs Timestamp to subtract from
  \param rhs Timeout to subtract
  \return Reference to decrement result
*/
inline Timeout& operator-=(Timeout& lhs, const Timeout& rhs)
{
	lhs = lhs - rhs;
	return lhs;
}
//! Multiplication operator
/*!
  \param lhs Timeout to multiply
  \param rhs Multiplier
  \return Multiplication result
*/
inline Timeout operator*(const Timeout& lhs, size_t rhs)
{
	return Timeout(lhs.timeSpec().tv_sec * rhs, lhs.timeSpec().tv_nsec * rhs);
}
//! Compound assignment multiplication operator
/*!
  \param lhs Timeout to multiply
  \param rhs Multiplier
  \return Reference to the result
*/
inline Timeout& operator*=(Timeout& lhs, size_t rhs)
{
	lhs = lhs * rhs;
	return lhs;
}
//! Division operator
/*!
  \param lhs Timeout to divide
  \param rhs Divisor
  \return Division result
*/
Timeout operator/(const Timeout& lhs, size_t rhs);
//! Compound assignment division operator
/*!
  \param lhs Timeout to divide
  \param rhs Divisor
  \return Reference to the result
*/
inline Timeout& operator/=(Timeout& lhs, size_t rhs)
{
	lhs = lhs / rhs;
	return lhs;
}

} // namespace isl

#endif

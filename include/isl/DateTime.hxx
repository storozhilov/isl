#ifndef ISL__DATE_TIME__HXX
#define ISL__DATE_TIME__HXX

#include <isl/TimeZone.hxx>
#include <isl/Timestamp.hxx>
#include <isl/Timeout.hxx>

namespace isl
{

//! Nanosecond-precision datetime
/*!
  A "human-readable Timestamp". :)

  UNIX datetime API is quite tricky and little difficult to use so I've invented another wheel,
  but this one has a relatively simple design. Basically, this is <tt>timespec</tt> value
  (GMT-second from Epoch and nanosecond) which has been broken down in <tt>struct tm</tt>
  UNIX breakdown time structure with partucular timezone applied to. This class is something
  like a wrapper for the libc datetime functions set to make them usable from C++.

  \sa TimeZone, Timestamp, Timeout
*/
class DateTime : public BasicDateTime
{
public:
	//! Default datetime format
	static const char * DefaultFormat;

	//! Constructs NULL datetime
	DateTime();
	//! Constructs datetime value
	/*!
	  \param year Year (there is no zero year)
	  \param month Month (1-12)
	  \param day Day of month (1-28/29/30/31)
	  \param hour Hour (0-23)
	  \param minute Minute (0-59)
	  \param second Second (0-59)
	  \param nanoSec Nanosecond (0-999999999)
	  \param tz Timezone
	*/
	DateTime(int year, int month, int day, int hour, int minute, int second, int nanoSec = 0, const TimeZone& tz = TimeZone::local());
	//! Constructs datetime value from UNIX breakdown time structure
	/*!
	  \param bdts UNIX breakdown time structure
	  \param nanoSec Nanosecond
	*/
	DateTime(const struct tm& bdts, int nanoSec = 0);
	//! Constructs datetime value from GMT-seconds from Epoch
	/*!
	  \param gmtSec GMT-seconds from Epoch
	  \param nanoSec Nanosecond
	  \param tz Timezone
	*/
	DateTime(time_t gmtSec, int nanoSec = 0, const TimeZone& tz = TimeZone::local());
	//! Constructs datetime value from POSIX.1b datetime representation
	/*!
	  \param ts POSIX.1b datetime representation
	  \param tz Timezone
	  \return TRUE if no error occured
	*/
	DateTime(const struct timespec& ts, const TimeZone& tz = TimeZone::local());
	//! Constructs datetime value from the timestamp
	/*!
	  \param timestamp Timestamp
	  \param tz Timezone
	  \return TRUE if no error occured
	*/
	DateTime(const Timestamp& timestamp, const TimeZone& tz = TimeZone::local());
	//! Constructs datetime value from the string using supplied format
	/*!
	  \param str String to parse
	  \param fmt Datetime format (see man strftime) including '\%f' for nanoseconds
	  \param tz Timezone for the date value
	  \return TRUE if the new datetime value is not NULL datetime
	*/
	DateTime(const std::string& str, const std::string& fmt = std::string(DefaultFormat), const TimeZone& tz = TimeZone::local());

	//! Inspects datetime for NULL time
	inline bool isNull() const
	{
		return _isNull;
	}
	//! Returns a GMT-second for datetime value
	inline time_t gmtSecond() const
	{
		return _ts.tv_sec;
	}
	//! Returns year
	inline int year() const
	{
		return isNull() ? 0 : _bdts.tm_year + 1900;
	}
	//! Returns month (1-12)
	inline int month() const
	{
		return isNull() ? 0 : _bdts.tm_mon + 1;
	}
	//! Returns day of month (1-28/29/30/31)
	inline int day() const
	{
		return isNull() ? 0 : _bdts.tm_mday;
	}
	//! Returns day of week (1-7, 1 is Sunday)
	inline int dayOfWeek() const
	{
		return isNull() ? 0 : _bdts.tm_wday + 1;
	}
	//! Returns day of year
	inline int dayOfYear() const
	{
		return isNull() ? 0 : _bdts.tm_yday + 1;
	}
	//! Returns hour
	inline int hour() const
	{
		return isNull() ? 0 : _bdts.tm_hour;
	}
	//! Returns minute
	inline int minute() const
	{
		return isNull() ? 0 : _bdts.tm_min;
	}
	//! Returns second
	inline int second() const
	{
		return isNull() ? 0 : _bdts.tm_sec;
	}
	//! Returns 
	inline int nanoSecond() const
	{
		return isNull() ? 0 : _ts.tv_nsec;
	}
	//! Returns timezone
	inline TimeZone tz() const
	{
		return isNull() ? TimeZone() : TimeZone(_bdts);
	}
	//! Returns UNIX break-down time structure representation of the datetime value
	/*!
	  \return UNIX break-down time structure representation of the datetime value
	*/
	inline const struct tm& bdts() const
	{
		return _bdts;
	}
	//! Returns POSIX.1b representation of the datetime value
	/*!
	  \return POSIX.1b representation for datetime
	*/
	inline const struct timespec& timeSpec() const
	{
		return _ts;
	}
	//! Formats datetime value with given format
	/*!
	    \param format Datetime format (see man strftime) including '\%f' for nanoseconds
	    \return Formatted datetime value
	*/
	std::string toString(const std::string& format = std::string(DefaultFormat)) const;
	//! Returns time part of the datetime value
	inline DateTime time() const
	{
		return DateTime(0, 0, 0, hour(), minute(), second(), nanoSecond(), tz());
	}
	//! Returns date part of the datetime value
	inline DateTime date() const
	{
		return DateTime(year(), month(), day(), 0, 0, 0, 0, tz());
	}
	//! Resets datetime value to the NULL one
	void reset();
	//! Sets datetime value
	/*!
	  \param year Year (there is no zero year)
	  \param month Month (1-12)
	  \param day Day of month (1-28/29/30/31)
	  \param hour Hour (0-23)
	  \param minute Minute (0-59)
	  \param second Second (0-59)
	  \param nanoSec Nanosecond (0-999999999)
	  \param tz Timezone
	  \return TRUE if no error occured
	*/
	bool set(int year, int month, int day, int hour, int minute, int second, int nanoSec = 0, const TimeZone& tz = TimeZone::local());
	//! Sets datetime value from UNIX breakdown time structure
	/*!
	  \param bdts UNIX breakdown time structure,
	  \param nanoSec Nanosecond
	  \return TRUE if no error occured
	*/
	inline bool set(const struct tm& bdts, int nanoSec = 0)
	{
		return set(bdts.tm_year + 1900, bdts.tm_mon + 1, bdts.tm_mday, bdts.tm_hour, bdts.tm_min, bdts.tm_sec, nanoSec, TimeZone(bdts));
	}
	//! Sets datetime value from GMT-seconds from Epoch
	/*!
	  \param gmtSec GMT-seconds from Epoch
	  \param nanoSec Nanosecond
	  \param tz Timezone
	  \return TRUE if no error occured
	*/
	bool set(time_t gmtSec, int nanoSec = 0, const TimeZone& tz = TimeZone::local());
	//! Sets datetime value from POSIX.1b datetime representation
	/*!
	  \param ts POSIX.1b datetime representation
	  \param tz Timezone
	  \return TRUE if no error occured
	*/
	inline bool set(const struct timespec& ts, const TimeZone& tz = TimeZone::local())
	{
		return set(ts.tv_sec, ts.tv_nsec, tz);
	}
	//! Sets datetime value from timestamp
	/*!
	  \param timestamp Timestamp
	  \param tz Timezone
	  \return TRUE if no error occured
	*/
	inline bool set(const Timestamp& timestamp, const TimeZone& tz = TimeZone::local())
	{
		return set(timestamp.timeSpec().tv_sec, timestamp.timeSpec().tv_nsec, tz);
	}
	//! Sets datetime from the string using supplied format
	/*!
	  \param str String to parse
	  \param fmt Datetime format (see man strftime) including '\%f' for nanoseconds
	  \param tz Timezone for the date value
	  \return TRUE if the new datetime value is not NULL datetime
	*/
	bool set(const std::string& str, const std::string& fmt = std::string(DefaultFormat), const TimeZone& tz = TimeZone::local());
	//! Comparison operator
	/*!
	  \param rhs Another datetime value to compare with
	*/
	inline bool operator==(const DateTime& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return isEqualsTo(rhs);
	}
	//! Comparison operator
	/*!
	  \param rhs Another datetime value to compare with
	*/
	inline bool operator!=(const DateTime& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return !isEqualsTo(rhs);
	}
	//! Comparison operator
	/*!
	  \param rhs Another datetime value to compare with
	*/
	inline bool operator<(const DateTime& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return isLessThen(rhs);
	}
	//! Comparison operator
	/*!
	  \param rhs Another datetime value to compare with
	*/
	inline bool operator<=(const DateTime& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return !rhs.isLessThen(*this);
	}
	//! Comparison operator
	/*!
	  \param rhs Another datetime value to compare with
	*/
	inline bool operator>(const DateTime& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return rhs.isLessThen(*this);
	}
	//! Comparison operator
	/*!
	  \param rhs Another datetime value to compare with
	*/
	inline bool operator>=(const DateTime& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return !isLessThen(rhs);
	}
	//! Addition operator
	/*!
	  \param rhs Timeout to be added
	  \return new datetime value with added timeout
	*/
	DateTime operator+(const Timeout& rhs) const;
	//! Datetime subtraction operator
	/*!
	  \param rhs Timeout to be subtracted
	  \return Timeout between this value and passed one
	*/
	Timeout operator-(const DateTime& rhs) const;
	//! Timeout subtraction operator
	/*!
	  \param rhs Timeout to be subtracted
	  \return New datetime value with subtracted timeout
	*/
	DateTime operator-(const Timeout& rhs) const;
	//! Increment operator
	/*!
	  \param rhs Timeout to be added
	  \return Reference to the datetime object
	*/
	DateTime& operator+=(const Timeout& rhs);
	//! Decrement operator
	/*!
	  \param rhs Timeout to be subtracted
	  \return Reference to the datetime object
	*/
	DateTime& operator-=(const Timeout& rhs);
	//! Returns current datetime
	/*!
	  \param tz Timezone of the result
	  \return Current datetime
	*/
	static DateTime now(const TimeZone& tz = TimeZone::local());
	//! Returns time limit (current timestamp plus timeout) as DateTime value
	/*inline static DateTime limit(const Timeout& timeout)
	{
		return DateTime(timeout.limit());
	}*/
	//! Returns an amount of interval expirations during the time frame
	/*!
	  Use this method in periodic job execution.

	  TODO Use big integer handling algorithms from Donald Knuth to avoid cycle an 24-hour interval limitation!

	  \param startedFrom Left boundary of the timeframe (included)
	  \param finishedBefore Right boundary of the timeframe (excluded)
	  \param interval Time interval
	*/
	//static size_t expirationsInFrame(const DateTime& startedFrom, const DateTime& finishedBefore, const Timeout& interval);
private:

	inline bool isEqualsTo(const DateTime& rhs) const
	{
		return ((gmtSecond() == rhs.gmtSecond()) && (nanoSecond() == rhs.nanoSecond()));
	}
	inline bool isLessThen(const DateTime& rhs) const
	{
		return gmtSecond() < rhs.gmtSecond() || (gmtSecond() == rhs.gmtSecond() && nanoSecond() < rhs.nanoSecond());
	}

	bool _isNull;
	struct timespec _ts;
	struct tm _bdts;
};

} // namespace isl

#endif


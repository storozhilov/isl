#ifndef ISL__TIME__HXX
#define ISL__TIME__HXX

#include <isl/String.hxx>
#include <string>
#include <time.h>

namespace isl
{

//! Nanosecond-precision time
class Time
{
public:
	//! Default time format (narrow characters)
	static const char * DefaultFormat;

	//! Constructs NULL time
	Time() :
		_isNull(true),
		_second(0),
		_nanoSecond(0),
		_gmtOffset(0)
	{}
	//! Constructs a time
	/*!
	  If an invalid data has been passed it constructs NULL time.

	  \param hour Hour
	  \param minute Minute
	  \param second Second
	  \param nanoSecond Nanosecond
	  \param gmtOffset Seconds east of UTC
	*/
	Time(int hour, int minute, int second = 0, int nanoSecond = 0, long int gmtOffset = 0) :
		_isNull(true),
		_second(0),
		_nanoSecond(0),
		_gmtOffset(0)
	{
		set(hour, minute, second, nanoSecond, gmtOffset);
	}
	//! Constructs time from the time_t value
	/*!
	  If an invalid data has been passed it constructs NULL time.

	  \param secondsFromEpoch Seconds from the Epoch (1970-01-01)
	  \param isLocalTime Does the time_t value should be treated as local time or GMT one
	  \param nanoSecond Nanoseconds
	*/
	Time(time_t secondsFromEpoch, bool isLocalTime, int nanoSecond = 0) :
		_isNull(true),
		_second(0),
		_nanoSecond(0),
		_gmtOffset(0)
	{
		set(secondsFromEpoch, isLocalTime, nanoSecond);
	}
	//! Constructs time from the UNIX break-down time structure
	/*!
	  If an invalid data has been passed it constructs NULL time.

	  \param bdts UNIX break-down time structure
	  \param nanoSecond Nanoseconds
	*/
	Time(const struct tm& bdts, unsigned int nanoSecond = 0) :
		_isNull(true),
		_second(0),
		_nanoSecond(0),
		_gmtOffset(0)
	{
		set(bdts, nanoSecond);
	}
	//! Constructs time from POSIX.1b structure for a time value
	/*!
	  If an invalid data has been passed it constructs NULL time.

	  \param ts POSIX.1b structure for a time value
	  \param isLocalTime Does the time value should be treated as local time or GMT one
	*/
	Time(const struct timespec& ts, bool isLocalTime) :
		_isNull(true),
		_second(0),
		_nanoSecond(0),
		_gmtOffset(0)
	{
		set(ts, isLocalTime);
	}
	//! Constructs time from the string using supplied format
	/*!
	  If an invalid data has been passed it constructs NULL time.

	  \param str String to parse
	  \param format Time format (see man strftime) including '%f' for nanoseconds
	*/
	Time(const std::string& str, const std::string& fmt = std::string(DefaultFormat)) :
		_isNull(true),
		_second(0),
		_nanoSecond(0),
		_gmtOffset(0)
	{
		set(str, fmt);
	}

	//! Inspects for NULL time
	inline bool isNull() const
	{
		return _isNull;
	}
	//! Returns hour or 0 if NULL time
	inline int hour() const
	{
		return isNull() ? 0 : _second / SecondsPerHour;
	}
	//! Returns minute or 0 if NULL time
	inline int minute() const
	{
		return isNull() ? 0 : (_second % SecondsPerHour) / SecondsPerMinute;
	}
	//! Returns second or 0 if NULL time
	inline int second() const
	{
		return isNull() ? 0 : _second % SecondsPerMinute;
	}
	//! Returns nanosecond or 0 if NULL time
	inline int nanoSecond() const
	{
		return isNull() ? 0 : _nanoSecond;
	}
	//! Returns GMT-offset in seconds
	inline int gmtOffset() const
	{
		return _gmtOffset;
	}
	//! Returns seconds since Epoch
	inline time_t toSecondsFromEpoch() const
	{
		return _second;
	}
	//! Converts time value to UNIX break-down time structure
	struct tm toBdts() const;
	//! Converts time value to POSIX.1b structure for a time value
	/*!
	  \return POSIX.1b representation for time
	*/
	inline struct timespec toTimeSpec() const
	{
		struct timespec ts;
		ts.tv_sec = isNull() ? 0 : _second;
		ts.tv_nsec = isNull() ? 0 : _nanoSecond;
		return ts;
	}
	//! Formats time value with given format
	/*!
	    \param format Time format (see man strftime) including '%f' for nanoseconds
	    \return Formatted time value
	*/
	std::string toString(const std::string& format = std::string(DefaultFormat)) const;
	//! Sets time
	/*!
	  \param hour Hour
	  \param minute Minute
	  \param second Second
	  \param nanoSecond Nanosecond
	  \param gmtOffset Seconds east of UTC
	  \return TRUE if the new time value is not NULL time
	*/
	bool set(int hour, int minute, int second, int nanoSecond = 0, long int gmtOffset = 0);
	//! Sets time from the time_t value
	/*!
	  \param secondsFromEpoch Seconds from the Epoch (1970-01-01)
	  \param isLocalTime Does the time value should be treated as local time or GMT one
	  \param nanoSecond Nanoseconds
	  \return TRUE if the new time value is not NULL time
	*/
	bool set(time_t secondsFromEpoch, bool isLocalTime, int nanoSecond = 0);
	//! Sets time from the UNIX break-down time structure
	/*!
	  \param bdts UNIX break-down time structure
	  \param nanoSecond Nanoseconds
	  \return TRUE if the new time value is not NULL time
	*/
	inline bool set(const struct tm& bdts, unsigned int nanoSecond = 0)
	{
		return set(bdts.tm_hour, bdts.tm_min, bdts.tm_sec, nanoSecond, bdts.tm_gmtoff);
	}
	//! Sets time from POSIX.1b structure for a time value
	/*!
	  \param ts POSIX.1b structure for a time value
	  \param isLocalTime Does the time value should be treated as local time or GMT one
	  \return TRUE if the new time value is not NULL time
	*/
	inline bool set(const struct timespec& ts, bool isLocalTime)
	{
		return set(ts.tv_sec, isLocalTime, ts.tv_nsec);
	}
	//! Sets time from the string using supplied format
	/*!
	  \param str String to parse
	  \param format Time format (see man strftime) including '%f' for nanoseconds
	  \return TRUE if the new time value is not NULL time
	*/
	bool set(const std::string& str, const std::string& fmt = std::string(DefaultFormat));
	//! Sets NULL time
	inline void reset()
	{
		_isNull = true;
		_second = 0;
		_nanoSecond = 0;
	}
	//! Returns copy of the object with added passed number of hours
	inline Time addHours(int nHours) const
	{
		return addSeconds(nHours * SecondsPerHour);
	}
	//! Returns copy of the object with added passed number of minutes
	inline Time addMinutes(int nMinutes) const
	{
		return addSeconds(nMinutes * SecondsPerMinute);
	}
	//! Returns copy of the object with added passed number of seconds
	/*!
	  \param nSeconds Number of seconds to add
	*/
	Time addSeconds(long int nSeconds) const;
	//! Returns copy of the object with added passed number of nanoseconds
	/*!
	  \param nNanoSeconds Number of nanoseconds to add
	*/
	Time addNanoSeconds(long int nNanoSeconds) const;
	/*//! Returns amount of nanoseconds to passed time
	int nanoSecondsTo(const Time& time) const;
	//! Returns amount of seconds to passed time
	int secondsTo(const Time& time) const;
	//! Sets time to the current one
	void start();
	//! Returns milliseconds elapsed from the time value. Sets time to the current one.
	int restart();
	//! Returns milliseconds elapsed from the time value.
	int elapsed() const;*/
	//! Comparence operator
	inline bool operator==(const Time& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return isEqualsTo(rhs);
	}
	//! Comparence operator
	inline bool operator!=(const Time& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return !isEqualsTo(rhs);
	}
	//! Comparence operator
	inline bool operator<(const Time& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return isLessThen(rhs);
	}
	//! Comparence operator
	inline bool operator<=(const Time& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return !rhs.isLessThen(*this);
	}
	//! Comparence operator
	inline bool operator>(const Time& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return rhs.isLessThen(*this);
	}
	//! Comparence operator
	inline bool operator>=(const Time& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return !isLessThen(rhs);
	}

	//! Returns current local time object
	static Time now();
	//! Validates time
	/*!
	    \param hour Hour
	    \param minute Minute
	    \param second Second
	    \param nanoSecond Nanosecond
	    \return true if arguments replresent a valid time value
	*/
	inline static bool isValid(int hour, int minute, int second, int nanoSecond = 0, long int gmtOffset = 0)
	{
		return (0 <= hour) && (hour <= 23) && (0 <= minute) && (minute <= 59) && (0 <= second) && (second <= 59) &&
			(0 <= nanoSecond) && (nanoSecond <= 999999999) && (gmtOffset >= 0) && (gmtOffset < SecondsPerDay);
	}
private:
	enum Consts {
		SecondsPerDay = 86400,
		SecondsPerHour = 3600,
		SecondsPerMinute = 60
	};

	Time(long int second, long int nanoSecond, long int gmtOffset);

	inline bool isEqualsTo(const Time& rhs) const
	{
		return _second == rhs._second && _nanoSecond == rhs._nanoSecond;
	}
	inline bool isLessThen(const Time& rhs) const
	{
		return _second < rhs._second || (_second == rhs._second && _nanoSecond < rhs._nanoSecond);
	}

	bool _isNull;
	long int _second;
	long int _nanoSecond;
	long int _gmtOffset;
	
	friend class DateTime;
};

} // namespace isl

#endif


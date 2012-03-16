#ifndef ISL__TIME__HXX
#define ISL__TIME__HXX

#include <isl/String.hxx>
#include <string>
#include <time.h>

namespace isl
{

//! Time representation class
/*!
    TODO Timezone names/abbreviations support
*/
class Time
{
public:
	//! Default time format (narrow characters)
	static const char * DefaultFormat;
	//! Default time format (wide characters)
	static const wchar_t * DefaultWFormat;

	//! Constructs NULL time
	Time();
	//! Constructs particular time
	/*!
	    \param hour Hour
	    \param minute Minute
	    \param second Second
	    \param millisecond Millisecond
	    \param gmtOffset Seconds east of UTC
	*/
	Time(int hour, int minute, int second = 0, int millisecond = 0, long int gmtOffset = 0);

	//! Inspects for NULL time
	inline bool isNull() const
	{
		return (_millisecond == NullTime);
	}
	//! Opposite of isNull()
	inline bool isValid() const
	{
		return !isNull();
	}
	//! Returns hour or 0 if NULL time
	inline int hour() const
	{
		return (isValid()) ? _millisecond / MillisecondsPerHour : 0;
	}
	//! Returns minute or 0 if NULL time
	inline int minute() const
	{
		return (isValid()) ? (_millisecond % MillisecondsPerHour) / MillisecondsPerMinute : 0;
	}
	//! Returns second or 0 if NULL time
	inline int second() const
	{
		return (isValid()) ? (_millisecond / 1000) % SecondsPerMinute : 0;
	}
	//! Returns millisecond or 0 if NULL time
	inline int msecond() const
	{
		return (isValid()) ? _millisecond % 1000 : 0;
	}
	//! Returns GMT-offset in seconds
	inline int gmtOffset() const
	{
		return _gmtOffset;
	}
	//! Formats time value to the one-byte character string
	/*!
	    \param format Time format (see man strftime) including '%f' for milliseconds
	    \return Formatted time value
	*/
	std::string toString(const std::string& format = std::string(DefaultFormat)) const;
	//! Formats time value to the wide character string
	/*!
	    \param format Time format (see man strftime) including '%f' for milliseconds
	    \return Formatted time value
	*/
	inline std::wstring toWString(const std::wstring& format = std::wstring(DefaultWFormat)) const
	{
		return String::utf8Decode(toString(String::utf8Encode(format)));
	}
	//! Converts time value to UNIX break-down time structure
	struct tm toBdts() const;
	//! Returns seconds since Epoch
	time_t secondsFromEpoch() const;
	//! Sets new time
	bool setTime(int hour, int minute, int second, int millisecond = 0);
	//! Sets NULL time
	inline void setNull()
	{
		_millisecond = NullTime;
	}
	//! Adds milliseconds
	Time addMSeconds(int nmseconds) const;
	//! Adds seconds
	inline Time addSeconds(int nseconds) const
	{
		return addMSeconds(nseconds * 1000);
	}
	//! Adds minutes
	inline Time addMinutes(int nminutes) const
	{
		return addMSeconds(nminutes * MillisecondsPerMinute);
	}
	//! Adds hours
	inline Time addHours(int nhours) const
	{
		return addMSeconds(nhours * MillisecondsPerHour);
	}
	//! Returns amount of milliseconds to passed time
	int msecondsTo(const Time& time) const;
	//! Returns amount of seconds to passed time
	int secondsTo(const Time& time) const;
	//! Sets time to the current one
	void start();
	//! Returns milliseconds elapsed from the time value. Sets time to the current one.
	int restart();
	//! Returns milliseconds elapsed from the time value.
	int elapsed() const;
	//! Comparence operator
	bool operator==(const Time& other) const;
	//! Comparence operator
	bool operator!=(const Time& other) const;
	//! Comparence operator
	bool operator<(const Time& other) const;
	//! Comparence operator
	bool operator<=(const Time& other) const;
	//! Comparence operator
	bool operator>(const Time& other) const;
	//! Comparence operator
	bool operator>=(const Time& other) const;

	//! Composes new Time object from time_t value
	/*!
	    \param nsecs Seconds from the Epoch (1970-01-01)
	    \param isLocalTime 'true' or 'false' if the time_t value should be treated as local time or GMT
	    \return new Date object
	*/
	static Time fromSecondsFromEpoch(time_t nsecs, bool isLocalTime);
	//! Returns current local time object
	static Time now();
	//! Validates time
	/*!
	    \param hour Hour
	    \param minute Minute
	    \param second Second
	    \param millisecond Millisecond
	    \return true if arguments replresent a valid time value
	*/
	static bool isValid(int hour, int minute, int second, int millisecond = 0);
	//! Parses time from narrow character string and returns new Time object from it
	/*!
	    \param str String to parse
	    \param format Time format (see man strftime) including '%f' for milliseconds
	    \return Time value
	*/
	static Time fromString(const std::string& str, const std::string& fmt = std::string(DefaultFormat));
	//! Parses time from wide character string and returns new Time object from it
	/*!
	    \param str String to parse
	    \param format Time format (see man strftime) including '%f' for milliseconds
	    \return Time value
	*/
	inline static Time fromWString(const std::wstring& str, const std::wstring& fmt = std::wstring(DefaultWFormat))
	{
		return fromString(String::utf8Encode(str), String::utf8Encode(fmt));
	}
	//! Composes new Time object from UNIX break-down time structure
	inline static Time fromBdts(struct tm& bdts, unsigned int msec = 0)
	{
		return Time(bdts.tm_hour, bdts.tm_min, bdts.tm_sec, msec, bdts.tm_gmtoff);
	}
private:
	enum Consts {
		NullTime = -1,
		SecondsPerDay = 86400,
		MillisecondsPerDay = 86400000,
		SecondsPerHour = 3600,
		MillisecondsPerHour = 3600000,
		SecondsPerMinute = 60,
		MillisecondsPerMinute = 60000,
		FormatBufferSize = 4096
	};

	int _millisecond;
	long int _gmtOffset;
	
	friend class DateTime;
};

} // namespace isl

#endif


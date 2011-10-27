#ifndef ISL__TIME__HXX
#define ISL__TIME__HXX

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
	//! Constructs NULL time
	Time();
	//! Constructs particular time
	/*!
	    \param hour Hour
	    \param minute Minute
	    \param second Second
	    \param millisecond Millisecond
	    \param gmtOffset Seconds east from GMT
	*/
	Time(int hour, int minute, int second = 0, int millisecond = 0, int gmtOffset = 0);

	//! Inspects for NULL time
	bool isNull() const;
	//! Opposite of isNull()
	bool isValid() const;
	//! Returns hour or 0 if NULL time
	int hour() const;
	//! Returns minute or 0 if NULL time
	int minute() const;
	//! Returns second or 0 if NULL time
	int second() const;
	//! Returns millisecond or 0 if NULL time
	int msecond() const;
	//! Returns GMT-offset in seconds
	inline int gmtOffset() const
	{
		return _gmtOffset;
	}
	//! Formats time value to the one-byte character string
	/*!
	    \param format Time format - see date(1)
	    TODO Milliseconds support
	*/
	std::string toString(const std::string& format = std::string(IsoOutputFormat)) const;
	//! Formats time value to the wide character string
	/*!
	    \param format Time format - see date(1)
	    TODO Milliseconds support
	*/
	std::wstring toWString(const std::wstring& format = std::wstring(IsoOutputWFormat)) const;
	//! Returns seconds since Epoch
	time_t secondsFromEpoch() const;
	//! Sets new time
	bool setTime(int hour, int minute, int second, int millisecond = 0);
	//! Sets NULL time
	void setNull();
	//! Adds milliseconds
	Time addMSeconds(int nmseconds) const;
	//! Adds seconds
	Time addSeconds(int nseconds) const;
	//! Adds minutes
	Time addMinutes(int nminutes) const;
	//! Adds hours
	Time addHours(int nhours) const;
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

	//! ISO time narrow character input format string
	static const char * IsoInputFormat;
	//! ISO time wide character input format string
	static const wchar_t * IsoInputWFormat;

	//! ISO time narrow character output format string
	static const char * IsoOutputFormat;
	//! ISO time wide character output format string
	static const wchar_t * IsoOutputWFormat;

	//! Composes new Date object from time_t value
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
	    TODO Milliseconds support
	*/
	static Time fromString(const std::string& str, const std::string& fmt = std::string(IsoInputFormat));
	//! Parses time from wide character string and returns new Time object from it
	/*!
	    TODO Milliseconds support
	*/
	static Time fromWString(const std::wstring& str, const std::wstring& fmt = std::wstring(IsoInputWFormat));

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
	int _gmtOffset;
	
	friend class DateTime;
};

} // namespace isl

#endif


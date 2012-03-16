#ifndef ISL__DATE_TIME__HXX
#define ISL__DATE_TIME__HXX

#include <isl/Date.hxx>
#include <isl/Time.hxx>

namespace isl
{

//! Date & time representation class
class DateTime
{
public:
	//! Default datetime format (narrow character)
	static const char * DefaultFormat;
	//! Default datetime format (wide characters)
	static const wchar_t * DefaultWFormat;

	//! Constructs NULL datetime
	DateTime();
	//! Constructs datetime value from the date value
	DateTime(const Date& date);
	//! Constructs datetime value from the date & time values
	DateTime(const Date& date, const Time& time);

	//! Inspects for NULL time
	inline bool isNull() const
	{
		return _date.isNull() || _time.isNull();
	}
	//! Opposite of isNull()
	inline bool isValid() const
	{
		return !isNull();
	}
	//! Returns date part of the datetime
	inline Date date() const
	{
		return _date;
	}
	//! Returns time part of the datetime
	inline Time time() const
	{
		return _time;
	}
	//! Sets date part of the datetime
	bool setDate(const Date& d);
	//! Sets time part of the datetime
	bool setTime(const Time& t);
	//! Sets NULL datetime
	inline void setNull()
	{
		_date.setNull();
		_time.setNull();
	}
	//! Returns copy of the object with added passed number of days
	/*!
	    \param ndays Number of day to add to the date
	*/
	DateTime addDays(int ndays) const;
	//! Returns copy of the object with added passed number of months
	/*!
	    \param nmonths Number of months to add to the date
	*/
	DateTime addMonths(int nmonths) const;
	//! Returns copy of the object with added passed number of years
	/*!
	    \param nyears Number of years to add to the date
	*/
	DateTime addYears(int nyears) const;
	//! Adds milliseconds
	DateTime addMSeconds(long nmseconds) const;
	//! Adds seconds
	DateTime addSeconds(int nseconds) const;
	//! Adds minutes
	DateTime addMinutes(int nminutes) const;
	//! Adds hours
	DateTime addHours(int nhours) const;
	//! Formats datetime value to the one-byte character string
	/*!
	    \param format Datetime format (see man strftime) including '%f' for milliseconds
	    \return Formatted datetime value
	*/
	std::string toString(const std::string& format = std::string(DefaultFormat)) const;
	//! Formats datetime value to the wide character string
	/*!
	    \param format Datetime format (see man strftime) including '%f' for milliseconds
	    \return Formatted datetime value
	*/
	inline std::wstring toWString(const std::wstring& format = std::wstring(DefaultWFormat)) const
	{
		return String::utf8Decode(toString(String::utf8Encode(format)));
	}
	//! Converts datetime value to UNIX break-down time structure
	struct tm toBdts() const;
	//! Returns seconds since Epoch
	inline time_t secondsFromEpoch() const
	{
		return _date.secondsFromEpoch() + _time.secondsFromEpoch();
	}
	//! Comparence operator
	bool operator==(const DateTime& other) const;
	//! Comparence operator
	bool operator!=(const DateTime& other) const;
	//! Comparence operator
	bool operator<(const DateTime& other) const;
	//! Comparence operator
	bool operator<=(const DateTime& other) const;
	//! Comparence operator
	bool operator>(const DateTime& other) const;
	//! Comparence operator
	bool operator>=(const DateTime& other) const;
	
	//! Composes new DateTime object from time_t value
	/*!
	    \param nsecs Seconds from the Epoch (1970-01-01)
	    \param isLocalTime 'true' or 'false' if the time_t value should be treated as local time or GMT
	    \return new Date object
	*/
	static DateTime fromSecondsFromEpoch(time_t nsecs, bool isLocalTime);
	//! Returns current datetime value for local time
	inline static DateTime now()
	{
		return DateTime(Date::now(), Time::now());
	}
	//! Parses datetime from narrow character string and returns new DateTime object from it
	/*!
	    \param str String to parse
	    \param format Datetime format (see man strftime) including '%f' for milliseconds
	    \return DateTime value
	*/
	static DateTime fromString(const std::string& str, const std::string& fmt = std::string(DefaultFormat));
	//! Parses datetime from wide character string and returns new DateTime object from it
	/*!
	    \param str String to parse
	    \param format Datetime format (see man strftime) including '%f' for milliseconds
	    \return DateTime value
	*/
	inline static DateTime fromWString(const std::wstring& str, const std::wstring& fmt = std::wstring(DefaultWFormat))
	{
		return fromString(String::utf8Encode(str), String::utf8Encode(fmt));
	}
	//! Composes new DateTime object from UNIX break-down time structure
	inline static DateTime fromBdts(struct tm& bdts, unsigned int msec = 0)
	{
		return DateTime(Date(bdts.tm_year + 1900, bdts.tm_mon + 1, bdts.tm_mday), Time(bdts.tm_hour, bdts.tm_min, bdts.tm_sec, msec, bdts.tm_gmtoff));
	}
private:
	enum Consts {
		FormatBufferSize = 4096
	};

	static bool str2bdts(const std::string& str, const std::string& fmt, struct tm& bdts, unsigned int& msec);
	static bool bdts2str(const struct tm& bdts, unsigned int msec, const std::string& fmt, std::string& str);

	Date _date;
	Time _time;

	friend class Time;
	friend class Date;
};

} // namespace isl

#endif


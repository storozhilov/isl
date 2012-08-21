#ifndef ISL__DATE__HXX
#define ISL__DATE__HXX

#include <isl/String.hxx>
#include <string>

namespace isl
{

//! Date
class Date
{
public:
	//! Default date format
	static const char * DefaultFormat;

	//! Constructs NULL date
	Date() :
		_dayNumber(0),
		_year(0),
		_month(0),
		_day(0)
	{}
	//! Constructs date
	/*!
	  If an invalid data has been passed it constructs NULL date.

	    \param year Year
	    \param month Month
	    \param day Day
	*/
	Date(int year, int month, int day) :
		_dayNumber(0),
		_year(0),
		_month(0),
		_day(0)
	{
		set(year, month, day);
	}
	//! Constructs date from the time_t value
	/*!
	  If an invalid data has been passed it constructs NULL date.

	  \param secondsFromEpoch Seconds from the Epoch (1970-01-01)
	  \param isLocalTime Does the time_t value should be treated as local time or GMT one
	*/
	Date(time_t secondsFromEpoch, bool isLocalTime) :
		_dayNumber(0),
		_year(0),
		_month(0),
		_day(0)
	{
		set(secondsFromEpoch, isLocalTime);
	};
	//! Constructs date from the UNIX break-down time structure
	/*!
	  If an invalid data has been passed it constructs NULL date.

	  \param bdts UNIX break-down time structure
	  \param nanoSecond Nanoseconds
	*/
	Date(const struct tm& bdts) :
		_dayNumber(0),
		_year(0),
		_month(0),
		_day(0)
	{
		set(bdts);
	}
	//! Constructs date from POSIX.1b structure for a time value
	/*!
	  If an invalid data has been passed it constructs NULL date.

	  \param ts POSIX.1b structure for a time value
	  \param isLocalTime Does the time value should be treated as local time or GMT one
	*/
	Date(const struct timespec& ts, bool isLocalTime) :
		_dayNumber(0),
		_year(0),
		_month(0),
		_day(0)
	{
		set(ts.tv_sec, isLocalTime);
	}
	//! Constructs date from the string using supplied format
	/*!
	  If an invalid data has been passed it constructs NULL date.

	  \param str String to parse
	  \param format Date format (see man strftime)
	*/
	Date(const std::string& str, const std::string& fmt = std::string(DefaultFormat)) :
		_dayNumber(0),
		_year(0),
		_month(0),
		_day(0)
	{
		set(str, fmt);
	}

	//! Inspects for NULL date
	inline bool isNull() const
	{
		return (_dayNumber == 0);
	}
	//! Returns day or 0 if NULL date
	inline int day() const
	{
		return _day;
	}
	//! Returns day of week (1..7) or 0 if NULL date
	/*!
	    \param mondayStartsWeek 'true' if the week starts from Monday, or 'false' if the week starts from Sunday
	    TODO Handle 'mondayStartsWeek' param!!!
	*/
	int dayOfWeek(bool mondayStartsWeek = true) const;
	//! Returns day of the year (1..365) or 0 if NULL date
	inline int dayOfYear() const
	{
		return isNull() ? 0 : (_dayNumber - Date(_year, 1, 1)._dayNumber + 1);
	}
	// Returns week number or 0 if NULL date
	/*!
	    \param mondayStartsWeek 'true' if the week starts from Monday, or 'false' if the week starts from Sunday
	*/
	inline int weekNumber(bool mondayStartsWeek = true) const
	{
		int actualYear;
		return weekNumber(actualYear, mondayStartsWeek);
	}
	// Returns week number with Sunday as start of the week or 0 if NULL date
	/*!
	    In accordance with ISO 8601, weeks start on Monday and the first Thursday of a year is always in week 1 of that year. Most years have 52 weeks, but some have 53.
	    \param actualYear Actual year of the week number (see ISO 8601) is is not always the same as year(). For example, 1 January 2000 has week number 52 in the year 1999, and 31 December 2002 has week number 1 in the year 2003.
	    \param mondayStartsWeek 'true' if the week starts from Monday, or 'false' if the week starts from Sunday
	    TODO Handle 'mondayStartsWeek' param!!!
	*/
	int weekNumber(int& actualYear, bool mondayStartsWeek = true) const;
	//! Returns days in month or 0 if NULL date
	inline int daysInMonth() const
	{
		return daysInMonth(_year, _month);
	}
	//! Returns days in year or 0 if NULL date
	inline int daysInYear() const
	{
		return daysInYear(_year);
	}
	//! Returns days to another date or 0 if NULL date
	/*!
	    \param date Another date
	*/
	inline int daysTo(const Date& date) const
	{
		return date._dayNumber - _dayNumber;
	}
	//! Returns month or 0 if NULL date
	inline int month() const
	{
		return _month;
	}
	//! Returns year or 0 if NULL date
	inline int year() const
	{
		return _year;
	}
	//! Sets date
	/*!
	  \param year Year
	  \param month Month
	  \param day Day
	  \return TRUE if the new time value is not NULL date
	*/
	bool set(int year, int month, int day);
	//! Sets date from the time_t value
	/*!
	  \param secondsFromEpoch Seconds from the Epoch (1970-01-01)
	  \param isLocalTime Does the time_t value should be treated as local time or GMT one
	  \return TRUE if the new time value is not NULL date
	*/
	bool set(time_t secondsFromEpoch, bool isLocalTime);
	//! Sets date from the UNIX break-down time structure
	/*!
	  \param bdts UNIX break-down time structure
	  \param nanoSecond Nanoseconds
	  \return TRUE if the new time value in not NULL date
	*/
	inline bool set(const struct tm& bdts)
	{
		return set(bdts.tm_year + 1900, bdts.tm_mon + 1, bdts.tm_mday);
	}
	//! Sets date from POSIX.1b structure for a time value
	/*!
	  \param ts POSIX.1b structure for a time value
	  \param isLocalTime Does the time value should be treated as local time or GMT one
	  \return TRUE if the new date value is not NULL date
	*/
	inline bool set(const struct timespec& ts, bool isLocalTime)
	{
		return set(ts.tv_sec, isLocalTime);
	}
	//! Sets date from the string using supplied format
	/*!
	  \param str String to parse
	  \param format Date format (see man strftime)
	  \return TRUE if the new time value is not NULL date
	*/
	bool set(const std::string& str, const std::string& fmt = std::string(DefaultFormat));
	//! Resets date to NULL value
	inline void reset()
	{
		_dayNumber = 0;
		_year = 0;
		_month = 0;
		_day = 0;
	}
	//! Returns copy of the object with added passed number of days
	/*!
	    \param ndays Number of day to add to the date
	*/
	Date addDays(int ndays) const;
	//! Returns copy of the object with added passed number of months
	/*!
	    \param nmonths Number of months to add to the date
	*/
	Date addMonths(int nmonths) const;
	//! Returns copy of the object with added passed number of years
	/*!
	    \param nyears Number of years to add to the date
	*/
	Date addYears(int nyears) const;
	//! Returns seconds since Epoch
	inline time_t toSecondsFromEpoch() const
	{
		return Date(1970, 1, 1).daysTo(*this) * SecondsPerDay;
	}
	//! Converts date value to UNIX break-down time structure
	struct tm toBdts() const;
	//! Converts date value to POSIX.1b structure for a time value
	/*!
	  \return POSIX.1b representation for date
	*/
	inline struct timespec toTimeSpec() const
	{
		struct timespec ts;
		ts.tv_sec = isNull() ? 0 : toSecondsFromEpoch();
		ts.tv_nsec = 0;
		return ts;
	}
	//! Formats date value to the one-byte character string
	/*!
	    \param format Date format (see man strftime)
	    \return Formatted date value
	*/
	std::string toString(const std::string& format = std::string(DefaultFormat)) const;
	//! Comparence operator
	inline bool operator==(const Date& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return _dayNumber == rhs._dayNumber;
	}
	//! Comparence operator
	inline bool operator!=(const Date& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return _dayNumber != rhs._dayNumber;
	}
	//! Comparence operator
	inline bool operator<(const Date& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return _dayNumber < rhs._dayNumber;
	}
	//! Comparence operator
	inline bool operator<=(const Date& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return _dayNumber <= rhs._dayNumber;
	}
	//! Comparence operator
	inline bool operator>(const Date& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return _dayNumber > rhs._dayNumber;
	}
	//! Comparence operator
	inline bool operator>=(const Date& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return _dayNumber >= rhs._dayNumber;
	}

	//! Returns true if leap year has been provided
	/*!
	    \param year Year
	*/
	static bool isLeapYear(int year);
	//! Returns true if parameters match a valid date
	/*!
	    \param year Year
	    \param month Month
	    \param day Day
	*/
	static bool isValid(int year, int month, int day);
	//! Returns days in year
	/*!
	    \param year Year
	*/
	static int daysInYear(int year);
	//! Returns days in month
	/*!
	    \param year Year
	    \param month Month
	*/
	static int daysInMonth(int year, int month);
	//! Returns date object for the current local tim
	static Date now();
private:
	enum Consts {
		FormatBufferSize = 4096,
		SecondsPerDay = 86400
	};
	
	const static int _monthDays[];

	static int dayNumberFromDate(int year, int month, int day);
	static void dateFromDayNumber(int dayNumber, int& year, int& month, int& day);

	int _dayNumber;
	int _year;
	int _month;
	int _day;

	friend class DateTime;
};

} // namespace isl

#endif

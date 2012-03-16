#ifndef ISL__DATE__HXX
#define ISL__DATE__HXX

#include <isl/String.hxx>
#include <string>

namespace isl
{

//! Date representation class
class Date
{
public:
	//! Default datetime format (narrow characters)
	static const char * DefaultFormat;
	//! ISO datetime format (wide characters)
	static const wchar_t * DefaultWFormat;

	//! Constructs NULL date
	Date();
	//! Constructs particular date
	/*!
	    \param year Year
	    \param month Month
	    \param day Day
	*/
	Date(int year, int month, int day);

	//! Inspects for NULL date
	inline bool isNull() const
	{
		return (_dayNumber == 0);
	}
	//! Opposite of isNull()
	inline bool isValid() const
	{
		return !isNull();
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
		return (_dayNumber - Date(_year, 1, 1)._dayNumber + 1);
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
	*/
	bool setDate(int year, int month, int day);
	//! Resets date to NULL value
	inline void setNull()
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
	//! Formats date value to the one-byte character string
	/*!
	    \param format Date format (see man strftime)
	    \return Formatted date value
	*/
	std::string toString(const std::string& format = std::string(DefaultFormat)) const;
	//! Formats date value to the wide character string
	/*!
	    \param format Date format (see man strftime)
	    \return Formatted date value
	*/
	inline std::wstring toWString(const std::wstring& format = std::wstring(DefaultWFormat)) const
	{
		return String::utf8Decode(toString(String::utf8Encode(format)));
	}
	//! Converts date value to UNIX break-down time structure
	struct tm toBdts() const;
	//! Returns seconds since Epoch
	time_t secondsFromEpoch() const;
	//! Comparence operator
	bool operator==(const Date& other) const;
	//! Comparence operator
	bool operator!=(const Date& other) const;
	//! Comparence operator
	bool operator<(const Date& other) const;
	//! Comparence operator
	bool operator<=(const Date& other) const;
	//! Comparence operator
	bool operator>(const Date& other) const;
	//! Comparence operator
	bool operator>=(const Date& other) const;

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
	//! Composes new Date object from time_t value
	/*!
	    \param nsecs Seconds from the Epoch (1970-01-01)
	    \param isLocalTime 'true' or 'false' if the time_t value should be treated as local time or GMT
	    \return new Date object
	*/
	static Date fromSecondsFromEpoch(time_t nsecs, bool isLocalTime);
	//! Returnse date object for the current local tim
	static Date now();
	//! Parses date from narrow character string and returns new Date object from it
	/*!
	    TODO Milliseconds support
	*/
	//! Parses date from narrow character string and returns new Date object from it
	/*!
	    \param str String to parse
	    \param format Date format (see man strftime)
	    \return Date value
	*/
	static Date fromString(const std::string& str, const std::string& fmt = std::string(DefaultFormat));
	//! Parses date from wide character string and returns new Date object from it
	/*!
	    \param str String to parse
	    \param format Date format (see man strftime)
	    \return Date value
	*/
	inline static Date fromWString(const std::wstring& str, const std::wstring& fmt = std::wstring(DefaultWFormat))
	{
		return fromString(String::utf8Encode(str), String::utf8Encode(fmt));
	}
	//! Composes new Date object from UNIX break-down time structure
	inline static Date fromBdts(struct tm& bdts)
	{
		return Date(bdts.tm_year + 1900, bdts.tm_mon + 1, bdts.tm_mday);
	}
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


#ifndef ISL__DATE__HXX
#define ISL__DATE__HXX

#include <string>

namespace isl
{

//! Date representation class
class Date
{
public:
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
	bool isNull() const;
	//! Opposite of isNull()
	bool isValid() const;
	//! Returns day or 0 if NULL date
	int day() const;
	//! Returns day of week (1..7) or 0 if NULL date
	/*!
	    \param mondayStartsWeek 'true' if the week starts from Monday, or 'false' if the week starts from Sunday
	    TODO Handle 'mondayStartsWeek' param!!!
	*/
	int dayOfWeek(bool mondayStartsWeek = true) const;
	//! Returns day of the year (1..365) or 0 if NULL date
	int dayOfYear() const;
	// Returns week number or 0 if NULL date
	/*!
	    \param mondayStartsWeek 'true' if the week starts from Monday, or 'false' if the week starts from Sunday
	*/
	int weekNumber(bool mondayStartsWeek = true) const;
	// Returns week number with Sunday as start of the week or 0 if NULL date
	/*!
	    In accordance with ISO 8601, weeks start on Monday and the first Thursday of a year is always in week 1 of that year. Most years have 52 weeks, but some have 53.
	    \param actualYear Actual year of the week number (see ISO 8601) is is not always the same as year(). For example, 1 January 2000 has week number 52 in the year 1999, and 31 December 2002 has week number 1 in the year 2003.
	    \param mondayStartsWeek 'true' if the week starts from Monday, or 'false' if the week starts from Sunday
	    TODO Handle 'mondayStartsWeek' param!!!
	*/
	int weekNumber(int& actualYear, bool mondayStartsWeek = true) const;
	//! Returns days in month or 0 if NULL date
	int daysInMonth() const;
	//! Returns days in year or 0 if NULL date
	int daysInYear() const;
	//! Returns days to another date or 0 if NULL date
	/*!
	    \param date Another date
	*/
	int daysTo(const Date& date) const;
	//! Returns month or 0 if NULL date
	int month() const;
	//! Returns year or 0 if NULL date
	int year() const;
	//! Sets date
	/*!
	    \param year Year
	    \param month Month
	    \param day Day
	*/
	bool setDate(int year, int month, int day);
	//! Resets date to NULL value
	void setNull();
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
	    \param format Date format - see date(1)
	    TODO Milliseconds support
	*/
	std::string toString(const std::string& format = std::string(IsoOutputFormat)) const;
	//! Formats date value to the wide character string
	/*!
	    \param format Date format - see date(1)
	    TODO Milliseconds support
	*/
	std::wstring toWString(const std::wstring& format = std::wstring(IsoOutputWFormat)) const;
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

	//! ISO datetime narrow character input format string
	static const char * IsoInputFormat;
	//! ISO datetime wide character input format string
	static const wchar_t * IsoInputWFormat;

	//! ISO datetime narrow character output format string
	static const char * IsoOutputFormat;
	//! ISO datetime wide character output format string
	static const wchar_t * IsoOutputWFormat;

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
	static Date fromString(const std::string& str, const std::string& fmt = std::string(IsoInputFormat));
	//! Parses date from wide character string and returns new Date object from it
	/*!
	    TODO Milliseconds support
	*/
	static Date fromWString(const std::wstring& str, const std::wstring& fmt = std::wstring(IsoInputWFormat));
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


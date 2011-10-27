#include <isl/Date.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <algorithm>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

namespace isl
{

/*------------------------------------------------------------------------------
 * Date
 * ---------------------------------------------------------------------------*/

const char * Date::IsoInputFormat = "%Y-%m-%d";
const wchar_t * Date::IsoInputWFormat = L"%Y-%m-%d";
const char * Date::IsoOutputFormat = "%Y-%m-%d";
const wchar_t * Date::IsoOutputWFormat = L"%Y-%m-%d";
const int Date::_monthDays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

Date::Date() :
	_dayNumber(0),
	_year(0),
	_month(0),
	_day(0)
{}

Date::Date(int year, int month, int day) :
	_dayNumber(0),
	_year(0),
	_month(0),
	_day(0)
{
	setDate(year, month, day);
}

bool Date::isNull() const
{
	return (_dayNumber == 0);
}

bool Date::isValid() const
{
	return !isNull();
}

int Date::day() const
{
	return _day;
}

int Date::dayOfWeek(bool mondayStartsWeek) const
{
	// TODO Handle 'mondayStartsWeek' param
	if (_dayNumber < 0) {
		return ((_dayNumber - 2) % 7) + 7;
	} else if (_dayNumber > 0) {
		return ((_dayNumber + 3) % 7) + 1;
	} else {
		return 0;
	}
}

int Date::dayOfYear() const
{
	return (_dayNumber - Date(_year, 1, 1)._dayNumber + 1);
}

int Date::weekNumber(bool mondayStartsWeek) const
{
	int actualYear;
	return weekNumber(actualYear, mondayStartsWeek);
}
	
int Date::weekNumber(int& actualYear, bool mondayStartsWeek) const
{
	// TODO Handle 'mondayStartsWeek' param
	if (isNull()) {
		return 0;
	}
	Date startOfThisYear(_year, 1, 1);
	int thisYearFirstWeekMondayDayNumber = startOfThisYear._dayNumber - startOfThisYear.dayOfWeek() + ((startOfThisYear.dayOfWeek() <= 4) ? 1 : 8);
	if (_dayNumber > thisYearFirstWeekMondayDayNumber) {
		Date startOfNextYear = startOfThisYear.addYears(1);
		int nextYearFirstWeekMondayDayNumber = startOfNextYear._dayNumber - startOfNextYear.dayOfWeek() +
			((startOfNextYear.dayOfWeek() <= 4) ? 1 : 8);
		if (_dayNumber >= nextYearFirstWeekMondayDayNumber) {
			actualYear = _year + 1;
			return 1;
		} else {
			actualYear = _year;
			return (_dayNumber - thisYearFirstWeekMondayDayNumber) / 7 + 1;
		}
	} else if (_dayNumber < thisYearFirstWeekMondayDayNumber) {
		Date startOfPreviousYear = startOfThisYear.addYears(-1);
		int previousYearFirstWeekMondayDayNumber = startOfPreviousYear._dayNumber - startOfPreviousYear.dayOfWeek() +
			((startOfPreviousYear.dayOfWeek() <= 4) ? 1 : 8);
		actualYear = _year - 1;
		return (_dayNumber - previousYearFirstWeekMondayDayNumber) / 7 + 1;
	} else {
		actualYear = _year;
		return 1;
	}
}

int Date::daysInMonth() const
{
	return daysInMonth(_year, _month);
}

int Date::daysInYear() const
{
	return daysInYear(_year);
}

int Date::daysTo(const Date& date) const
{
	return date._dayNumber - _dayNumber;
}

int Date::month() const
{
	return _month;
}

int Date::year() const
{
	return _year;
}

bool Date::setDate(int year, int month, int day)
{
	_dayNumber = dayNumberFromDate(year, month, day);
	if (_dayNumber == 0) {
		_year = 0;
		_month = 0;
		_day = 0;
		return false;
	} else {
		_year = year;
		_month = month;
		_day = day;
		return true;
	}
}

void Date::setNull()
{
	_dayNumber = 0;
	_year = 0;
	_month = 0;
	_day = 0;
}

Date Date::addDays(int ndays) const
{
	if (isNull() || (ndays == 0)) {
		return *this;
	}
	Date newDate(*this);
	newDate._dayNumber += ndays;
	if (_dayNumber > 0 && (newDate._dayNumber <= 0)) {
		--newDate._dayNumber;
	} else if (_dayNumber < 0 && (newDate._dayNumber >= 0)) {
		++newDate._dayNumber;
	}
	dateFromDayNumber(newDate._dayNumber, newDate._year, newDate._month, newDate._day);
	return newDate;
}

Date Date::addMonths(int nmonths) const
{
	if (isNull() || (nmonths == 0)) {
		return *this;
	}
	bool increasing = (nmonths > 0);
	int y = _year;
	int m = _month;
	int d = _day;
	// TODO Test code below
	while (nmonths != 0) {
		if (nmonths < 0 && nmonths + 12 <= 0) {
			--y;
			nmonths += 12;
		} else if (nmonths < 0) {
			m += nmonths;
			nmonths = 0;
			if (m <= 0) {
				--y;
				m += 12;
			}
		} else if (nmonths - 12 >= 0) {
			++y;
			nmonths -= 12;
		} else if (m == 12) {
			++y;
			m = 0;
		} else {
			m += nmonths;
			nmonths = 0;
			if (m > 12) {
				++y;
				m -= 12;
			}
		}
	}
	if ((_year > 0 && y <= 0) || (_year < 0 && y >= 0)) {
		y += (increasing) ? +1 : -1;
	}
	if (y == 1582 && m == 10 && d > 4 && d < 15) {
		d = (increasing) ? 15 : 4;
	}
	return Date(y, m, std::min(d, daysInMonth(y, m)));
}

Date Date::addYears(int nyears) const
{
	if (isNull() || (nyears == 0)) {
		return *this;
	}
	int y = _year + nyears;
	if ((_year > 0 && y <= 0) || (_year < 0 && y >= 0)) {
		y += (nyears > 0) ? +1 : -1;
	}
	return Date(y, _month, std::min(_day, daysInMonth(y, _month)));
}

std::string Date::toString(const std::string& format) const
{
	if (isNull()) {
		return "[null]";
	}
	tm bdt;
	bdt.tm_sec = 0;
	bdt.tm_min = 0;
	bdt.tm_hour = 0;
	bdt.tm_mday = day();
	bdt.tm_mon = month() - 1;
	bdt.tm_year = year() - 1900;
	bdt.tm_wday = dayOfWeek(false) - 1;
	bdt.tm_yday = dayOfYear() - 1;
	bdt.tm_isdst = 0;
	bdt.tm_gmtoff = 0;
	bdt.tm_zone = 0;
	char buf[FormatBufferSize];
	size_t len = strftime(buf, FormatBufferSize, format.c_str(), &bdt);
	if (len <= 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::StrFTime, errno, L"Time formatting error"));
	}
	return std::string(buf, len);
}

std::wstring Date::toWString(const std::wstring& format) const
{
	return String::utf8Decode(toString(String::utf8Encode(format)));
}

time_t Date::secondsFromEpoch() const
{
	Date epoch(1970, 1, 1);
	return epoch.daysTo(*this) * SecondsPerDay;
}

bool Date::operator==(const Date& other) const
{
	return _dayNumber == other._dayNumber;
}

bool Date::operator!=(const Date& other) const
{
	return _dayNumber != other._dayNumber;
}

bool Date::operator<(const Date& other) const
{
	return _dayNumber < other._dayNumber;
}

bool Date::operator<=(const Date& other) const
{
	return _dayNumber <= other._dayNumber;
}

bool Date::operator>(const Date& other) const
{
	return _dayNumber > other._dayNumber;
}

bool Date::operator>=(const Date& other) const
{
	return _dayNumber >= other._dayNumber;
}

bool Date::isLeapYear(int year)
{
	if (year < 1582) {
		return (abs(year) % 4 == 0);
	} else {
		return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
	}
}

bool Date::isValid(int year, int month, int day)
{
	if (year == 0) {
		return false;
	}
	// Passage from Julian to Gregorian calendar
	if (year == 1582 && month == 10 && day > 4 && day < 15) {
		return false;
	}
	return (day > 0 && month > 0 && month <= 12) &&
		(day <= _monthDays[month] || (day == 29 && month == 2 && isLeapYear(year)));
}

int Date::daysInYear(int year)
{
	if (!isValid(year, 1, 1)) {
		return 0;
	} else if (year == 1582) {
		return 356;
	} else {
		return (isLeapYear(year)) ? 366 : 365;
	}
}

int Date::daysInMonth(int year, int month)
{
	if (!isValid(year, month, 1)) {
		return 0;
	} else if (year == 1582 && month == 10) {
		return 21;		
	} else {
		return (month == 2 && isLeapYear(year)) ? 29 : _monthDays[month];
	}
}

Date Date::fromSecondsFromEpoch(time_t nsecs, bool isLocalTime)
{
	tm bdt;
	if (isLocalTime) {
		if (localtime_r(&nsecs, &bdt) == NULL) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::LocalTimeR, errno, L"Error converting time_t to local time"));
		}
	} else {
		if (gmtime_r(&nsecs, &bdt) == NULL) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::LocalTimeR, errno, L"Error converting time_t to GMT"));
		}
	}
	return Date(bdt.tm_year + 1900, bdt.tm_mon + 1, bdt.tm_mday);
}

Date Date::now()
{
	struct timeval tv;
	struct timezone tz;
	if (gettimeofday(&tv, &tz) != 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::GetTimeOfDay, errno, L"Fetching time of day error"));
	}
	tm bdt;
	if (localtime_r(&(tv.tv_sec), &bdt) == NULL) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::LocalTimeR, errno, L"Error converting time_t to local time"));
	}
	return Date(bdt.tm_year + 1900, bdt.tm_mon + 1, bdt.tm_mday);
}

Date Date::fromString(const std::string& str, const std::string& fmt)
{
	tm bdt;
	memset(&bdt, 0, sizeof(struct tm));
	// TODO Correct parsing error handling!
	if (strptime(str.c_str(), fmt.c_str(), &bdt) == NULL) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::StrPTime, errno, L"Error parsing date value"));
	}
	return Date(bdt.tm_year + 1900, bdt.tm_mon + 1, bdt.tm_mday);
}

Date Date::fromWString(const std::wstring& str, const std::wstring& fmt)
{
	return fromString(String::utf8Encode(str), String::utf8Encode(fmt));
}

int Date::dayNumberFromDate(int year, int month, int day)
{
	// NOTE:
	// -1-th day number is 31.12.-0001
	// 0-th day number is null/invalid date
	// 1-th day number is 01.01.0001
	// 577737-th day number is 04.10.1582 (End of the Julian calendar)
	// 577738-th day number is 15.10.1582 (Beginning of the Gregorian calendar)
	if (!isValid(year, month, day)) {
		return 0;
	}
	int result = 0;
	if (year > 0) {
		// Adding years
		for (int i = 1; i < year; ++i) {
			result += daysInYear(i);
		}
		// Adding months
		for (int i = 1; i < month; ++i) {
			result += daysInMonth(year, i);
		}
		// Adding days
		result += (year == 1582 && month == 10 && day > 4) ? day - 10 : day;
	} else {
		// Subtracting years
		for (int i = -1; i > year; --i) {
			result -= daysInYear(i);
		}
		// Subtracting months
		for (int i = 12; i > month; --i) {
			result -= daysInMonth(year, i);
		}
		// Subtracting days
		result -= ((daysInMonth(year, month) - day) + 1);
	}
	return result;
}

void Date::dateFromDayNumber(int dayNumber, int& year, int& month, int& day)
{
	// NOTE:
	// -1-th day number is 31.12.-0001
	// 0-th day number is null/invalid date
	// 1-th day number is 01.01.0001
	// 577737-th day number is 04.10.1582 (End of the Julian calendar)
	// 577738-th day number is 15.10.1582 (Beginning of the Gregorian calendar)
	if (dayNumber > 0) {
		// Calculating year
		year = 1;
		while (dayNumber > daysInYear(year)) {
			dayNumber -= daysInYear(year);
			++year;
		}
		// Calculating month
		month = 1;
		while (dayNumber > daysInMonth(year, month)) {
			dayNumber -= daysInMonth(year, month);
			++month;
		}
		// Calculating day
		day = (year == 1582 && month == 10 && dayNumber > 4) ? dayNumber + 10 : dayNumber;
	} else if (dayNumber < 0) {
		// Calculating year
		year = -1;
		while (dayNumber < -daysInYear(year)) {
			dayNumber += daysInYear(year);
			--year;
		}
		// Calculating month
		month = 12;
		while (dayNumber < -daysInMonth(year, month)) {
			dayNumber += daysInMonth(year, month);
			--month;
		}
		// Calculating day
		day = daysInMonth(year, month) + dayNumber + 1;
	} else {
		year = 0;
		month = 0;
		day = 0;
	}
}

} // namespace isl


#include <isl/DateTime.hxx>
#include <sstream>
#include <iomanip>
#include <sys/time.h>

#include <iostream>

namespace isl
{

/*------------------------------------------------------------------------------
 * DateTime
 * ---------------------------------------------------------------------------*/

DateTime::DateTime() :
	_date(),
	_time()
{}

DateTime::DateTime(const Date& date) :
	_date(date),
	_time()
{
	if (_date.isNull()) {
		setNull();
	}
}

DateTime::DateTime(const Date& date, const Time& time) :
	_date(date),
	_time(time)
{
	if (_date.isNull() || _time.isNull()) {
		setNull();
	}
}

DateTime::DateTime(time_t t) :
	_date(),
	_time()
{
	setTime_t(t);
}

bool DateTime::isNull() const
{
	return _date.isNull() || _time.isNull();
}

bool DateTime::isValid() const
{
	return !isNull();
}

Date DateTime::date() const
{
	return _date;
}

Time DateTime::time() const
{
	return _time;
}

bool DateTime::setDate(const Date& d)
{
	if (d.isNull()) {
		setNull();
		return false;
	}
	_date = d;
	if (_time.isNull()) {
		_time.setTime(0, 0, 0);
	}
	return true;
}

bool DateTime::setTime(const Time& t)
{
	if (isNull()) {
		return false;
	}
	if (t.isNull()) {
		setNull();
		return false;
	}
	_time = t;
	return true;
}

void DateTime::setNull()
{
	_date.setNull();
	_time.setNull();
}

time_t DateTime::toTime_t() const
{
	int days = Date(1970, 1, 1).daysTo(_date);
	std::cout << "days = " << days << std::endl;
	int seconds = Time().secondsTo(_time);
	std::cout << "seconds = " << seconds << std::endl;
	if (days < 0 || (days == 0 && seconds < 0)) {
		return -1;
	}
	return (static_cast<time_t>(days) * Time::SecondsPerDay) + seconds;
}

void DateTime::setTime_t(time_t t)
{
	if (t < 0) {
		setNull();
	}
	_date = Date(1970, 1, 1).addDays(t / Time::SecondsPerDay);
	_time = Time().addSeconds(t % Time::SecondsPerDay);
}

DateTime DateTime::addDays(int ndays) const
{
	if (isNull() || (ndays == 0)) {
		return *this;
	}
	return DateTime(_date.addDays(ndays), _time);
}

DateTime DateTime::addMonths(int nmonths) const
{
	if (isNull() || (nmonths == 0)) {
		return *this;
	}
	return DateTime(_date.addMonths(nmonths), _time);
}

DateTime DateTime::addYears(int nyears) const
{
	if (isNull() || (nyears == 0)) {
		return *this;
	}
	return DateTime(_date.addYears(nyears), _time);
}

DateTime DateTime::addMSeconds(long nmseconds) const
{
	if (isNull() || (nmseconds == 0)) {
		return *this;
	}
	int ndays;
	if (nmseconds < 0) {
		ndays = ((nmseconds + _time._millisecond) < 0) ? (nmseconds + _time._millisecond) / Time::MillisecondsPerDay - 1 : 0;
	} else {
		ndays = (nmseconds + _time._millisecond) / Time::MillisecondsPerDay;
	}
	return DateTime(_date.addDays(ndays), _time.addMSeconds(nmseconds));
}

DateTime DateTime::addSeconds(int nseconds) const
{
	if (isNull() || (nseconds == 0)) {
		return *this;
	}
	return addMSeconds(static_cast<long>(nseconds) * 1000);
}

DateTime DateTime::addMinutes(int nminutes) const
{
	if (isNull() || (nminutes == 0)) {
		return *this;
	}
	return addMSeconds(static_cast<long>(nminutes) * Time::MillisecondsPerMinute);
}

DateTime DateTime::addHours(int nhours) const
{
	if (isNull() || (nhours == 0)) {
		return *this;
	}
	return addMSeconds(static_cast<long>(nhours) * Time::MillisecondsPerHour);
}

std::wstring DateTime::toString(const std::wstring& format) const
{
	if (isNull()) {
		return L"null";
	}
	Formatter formatter(*this);
	FormattedWString<Formatter> fs(formatter, &Formatter::substitute, format);
	return fs.str();
}

std::wstring DateTime::toGMT() const
{
	return toString(L"%2W, %1D %2M %2Y %1h:%1m:%1s GMT");
}

bool DateTime::operator==(const DateTime& other) const
{
	if (isNull() || other.isNull()) {
		return false;
	}
	return ((_date == other._date) && (_time == other._time));
}

bool DateTime::operator!=(const DateTime& other) const
{
	if (isNull() || other.isNull()) {
		return false;
	}
	return ((_date != other._date) || (_time != other._time));
}

bool DateTime::operator<(const DateTime& other) const
{
	if (isNull() || other.isNull()) {
		return false;
	}
	return ((_date < other._date) || ((_date == other._date) && (_time < other._time)));
}

bool DateTime::operator<=(const DateTime& other) const
{
	if (isNull() || other.isNull()) {
		return false;
	}
	return ((_date < other._date) || ((_date == other._date) && (_time <= other._time)));
}

bool DateTime::operator>(const DateTime& other) const
{
	if (isNull() || other.isNull()) {
		return false;
	}
	return ((_date > other._date) || ((_date == other._date) && (_time > other._time)));
}

bool DateTime::operator>=(const DateTime& other) const
{
	if (isNull() || other.isNull()) {
		return false;
	}
	return ((_date > other._date) || ((_date == other._date) && (_time >= other._time)));
}
	
DateTime DateTime::now()
{
	return DateTime(Date::now(), Time::now());
}

/*------------------------------------------------------------------------------
 * DateTime::Formatter
 * ---------------------------------------------------------------------------*/

DateTime::Formatter::Formatter(const DateTime& datetime) :
	_datetime(datetime)
{}

std::wstring DateTime::Formatter::substitute(wchar_t fmt, const std::wstring& param)
{
	std::wostringstream sstr;
	switch (fmt) {
		case L'D':
			if (param == L"1") {						// Day of month with leading zero
				sstr << std::setfill(L'0') << std::setw(2) << _datetime._date.day();
			} else if (param.empty()) {					// Day of month
				sstr << _datetime._date.day();
			} else {
				sstr << L"[Unknown format parameters: '" << param << L"']";
			}
			break;
		case L'M':
			if (param == L"1") {						// Month number with leading zero
				sstr << std::setfill(L'0') << std::setw(2) << _datetime._date.month();
			} else if (param == L"2") {					// Month GMT name
				sstr << Date::monthGMTNames[_datetime._date.month()];
			} else if (param.empty()) {					// Month number
				sstr << _datetime._date.month();
			} else {
				sstr << L"[Unknown format parameters: '" << param << L"']";
			}
			break;
		case L'Y':
			if (param == L"1") {						// Year number as two digits with leading zero
				if (_datetime._date.year() >= 2000 && _datetime._date.year() < 2100) {
					sstr << std::setfill(L'0') << std::setw(2) << (_datetime._date.year() / 2);
				} else {
					sstr << _datetime._date.year();
				}
			} else if (param == L"2") {					// Year number as for digits with leading zeros
				sstr << std::setfill(L'0') << std::setw(4) << _datetime._date.year();
			} else if (param.empty()) {					// Year number
				sstr << _datetime._date.year();
			} else {
				sstr << L"[Unknown format parameters: '" << param << L"']";
			}
			break;
		case L'W':
			if (param == L"1") {						// Week number
				sstr << _datetime._date.weekNumber();
			} else if (param == L"2") {					// Day of week GMT name
				sstr << Date::daysOfWeekGMTNames[_datetime._date.dayOfWeek()];
			} else if (param.empty()) {					// Day of week number
				sstr << _datetime._date.dayOfWeek();
			} else {
				sstr << L"[Unknown format parameters: '" << param << L"']";
			}
			break;
		case L'h':
			if (param == L"1") {						// Hour with leading zero
				sstr << std::setfill(L'0') << std::setw(2) << _datetime._time.hour();
			} else if (param.empty()) {					// Hour
				sstr << _datetime._time.hour();
			} else {
				sstr << L"[Unknown format parameters: '" << param << L"']";
			}
			break;
		case L'm':
			if (param == L"1") {						// Minute with leading zero
				sstr << std::setfill(L'0') << std::setw(2) << _datetime._time.minute();
			} else if (param.empty()) {					// Minute
				sstr << _datetime._time.minute();
			} else {
				sstr << L"[Unknown format parameters: '" << param << L"']";
			}
			break;
		case L's':
			if (param == L"1") {						// Second with leading zero
				sstr << std::setfill(L'0') << std::setw(2) << _datetime._time.second();
			} else if (param.empty()) {					// Second
				sstr << _datetime._time.second();
			} else {
				sstr << L"[Unknown format parameters: '" << param << L"']";
			}
			break;
		case L'z':
			if (param == L"1") {						// Milliseconds with two leading zeros
				sstr << std::setfill(L'0') << std::setw(3) << _datetime._time.msecond();
			} else if (param.empty()) {					// Milliseconds
				sstr << _datetime._time.msecond();
			} else {
				sstr << L"[Unknown format parameters: '" << param << L"']";
			}
			break;
		default:
			sstr << L"[Unknown format symbol: '" << fmt << L"']";
	}
	return sstr.str();
}

} // namespace isl


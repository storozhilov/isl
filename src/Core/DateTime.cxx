#include <isl/DateTime.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <sys/time.h>
#include <errno.h>

#include <iostream>

namespace isl
{

/*------------------------------------------------------------------------------
 * DateTime
 * ---------------------------------------------------------------------------*/

const char * DateTime::HttpInputFormat = "%a, %d %b %Y %H:%M:%S %Z";
const wchar_t * DateTime::HttpInputWFormat = L"%a, %d %b %Y %H:%M:%S %Z";
const char * DateTime::HttpOutputFormat = "%a, %d %b %Y %H:%M:%S %Z";
const wchar_t * DateTime::HttpOutputWFormat = L"%a, %d %b %Y %H:%M:%S %Z";

const char * DateTime::IsoInputFormat = "%Y-%m-%d %H:%M:%S";
const wchar_t * DateTime::IsoInputWFormat = L"%Y-%m-%d %H:%M:%S";
const char * DateTime::IsoOutputFormat = "%Y-%m-%d %H:%M:%S";
const wchar_t * DateTime::IsoOutputWFormat = L"%Y-%m-%d %H:%M:%S";

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

std::string DateTime::toString(const std::string& format) const
{
	if (isNull()) {
		return "[null]";
	}
	tm bdt;
	bdt.tm_sec = _time.second();
	bdt.tm_min = _time.minute();
	bdt.tm_hour = _time.hour();
	bdt.tm_mday = _date.day();
	bdt.tm_mon = _date.month() - 1;
	bdt.tm_year = _date.year() - 1900;
	bdt.tm_wday = _date.dayOfWeek(false) - 1;
	bdt.tm_yday = _date.dayOfYear() - 1;
	bdt.tm_isdst = 0;
	bdt.tm_gmtoff = _time.gmtOffset();
	bdt.tm_zone = 0;
	char buf[FormatBufferSize];
	size_t len = strftime(buf, FormatBufferSize, format.c_str(), &bdt);
	if (len <= 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::StrFTime, errno, L"Time formatting error"));
	}
	return std::string(buf, len);
}

std::wstring DateTime::toWString(const std::wstring& format) const
{
	return String::utf8Decode(toString(String::utf8Encode(format)));
}

time_t DateTime::secondsFromEpoch() const
{
	return _date.secondsFromEpoch() + _time.secondsFromEpoch();
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

DateTime DateTime::fromSecondsFromEpoch(time_t nsecs, bool isLocalTime)
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
	return DateTime(Date(bdt.tm_year + 1900, bdt.tm_mon + 1, bdt.tm_mday), Time(bdt.tm_hour, bdt.tm_min, bdt.tm_sec, bdt.tm_gmtoff));
}

DateTime DateTime::now()
{
	return DateTime(Date::now(), Time::now());
}

DateTime DateTime::fromString(const std::string& str, const std::string& fmt)
{
	tm bdt;
	memset(&bdt, 0, sizeof(struct tm));
	// TODO Correct parsing error handling!
	if (strptime(str.c_str(), fmt.c_str(), &bdt) == NULL) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::StrPTime, errno, L"Error parsing time value"));
	}
	return DateTime(Date(bdt.tm_year + 1900, bdt.tm_mon + 1, bdt.tm_mday), Time(bdt.tm_hour, bdt.tm_min, bdt.tm_sec, bdt.tm_gmtoff));
}

DateTime DateTime::fromWString(const std::wstring& str, const std::wstring& fmt)
{
	return fromString(String::utf8Encode(str), String::utf8Encode(fmt));
}

} // namespace isl


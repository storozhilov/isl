#include <isl/common.hxx>
#include <isl/DateTime.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/LogMessage.hxx>
#include <sys/time.h>
#include <errno.h>
#include <iomanip>

#include <iostream>

namespace isl
{

/*------------------------------------------------------------------------------
 * DateTime
 * ---------------------------------------------------------------------------*/

const char * DateTime::DefaultFormat = "%Y-%m-%d %H:%M:%S.%f";
const wchar_t * DateTime::DefaultWFormat = L"%Y-%m-%d %H:%M:%S.%f";

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
	std::string result;
	if (!DateTime::bdts2str(toBdts(), _time.msecond(), format, result)) {
		result.assign("[invalid datetime format]");
	}
	return result;
}

struct tm DateTime::toBdts() const
{
	struct tm bdts;
	memset(&bdts, 0, sizeof(struct tm));
	bdts.tm_sec = _time.second();
	bdts.tm_min = _time.minute();
	bdts.tm_hour = _time.hour();
	bdts.tm_gmtoff = _time._gmtOffset;
	bdts.tm_mday = _date.day();
	bdts.tm_mon = _date.month() - 1;
	bdts.tm_year = _date.year() - 1900;
	bdts.tm_wday = _date.dayOfWeek(false) - 1;
	bdts.tm_yday = _date.dayOfYear() - 1;
	return bdts;
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
	tm bdts;
	if (isLocalTime) {
		if (localtime_r(&nsecs, &bdts) == NULL) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::LocalTimeR, errno, "Error converting time_t to local time"));
		}
	} else {
		if (gmtime_r(&nsecs, &bdts) == NULL) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::LocalTimeR, errno, "Error converting time_t to GMT"));
		}
	}
	return DateTime(Date(bdts.tm_year + 1900, bdts.tm_mon + 1, bdts.tm_mday), Time(bdts.tm_hour, bdts.tm_min, bdts.tm_sec, bdts.tm_gmtoff));
}

DateTime DateTime::fromString(const std::string& str, const std::string& fmt)
{
	struct tm bdts;
	unsigned int msec;
	if (!DateTime::str2bdts(str, fmt, bdts, msec)) {
		return DateTime();
	}
	return fromBdts(bdts, msec);
}

bool DateTime::str2bdts(const std::string& str, const std::string& fmt, struct tm& bdts, unsigned int& msec)
{
	memset(&bdts, 0, sizeof(struct tm));
	msec = 0;
	size_t fmtStartPos = 0;
	const char * strPtr = str.c_str();
	size_t millisecondFmtPos;
	do {
		millisecondFmtPos = fmt.find("%f", fmtStartPos);
		std::string fmtPart = fmt.substr(fmtStartPos, millisecondFmtPos == std::string::npos ? std::string::npos : millisecondFmtPos - fmtStartPos);
		strPtr = strptime(strPtr, fmtPart.c_str(), &bdts);
		if (!strPtr) {
			std::ostringstream msg;
			msg << "Error parsing \"" << str << "\" string using \"" << fmt << "\" format with strptime(3) system call";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			return false;
		}
		if (millisecondFmtPos != std::string::npos) {
			for (unsigned int i = 0; i < 3; ++i) {
				if (*strPtr == '\0') {
					std::ostringstream msg;
					msg << "Error parsing \"" << str << "\" string using \"" << fmt << "\" format: premature end of milliseconds value";
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
					return false;
				}
				if (*strPtr < '0' || *strPtr > '9') {
					std::ostringstream msg;
					msg << "Error parsing \"" << str << "\" string using \"" << fmt << "\" format: invalid milliseconds value";
					debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
					return false;
				}
				msec = msec * 10 + *strPtr - '0';
				++strPtr;
			}
			fmtStartPos = millisecondFmtPos + 2;
		}
	} while (millisecondFmtPos != std::string::npos && fmtStartPos < fmt.length());
	return true;
}

bool DateTime::bdts2str(const struct tm& bdts, unsigned int msec, const std::string& fmt, std::string& str)
{
	str.clear();
	char buf[FormatBufferSize];
	size_t len = strftime(buf, FormatBufferSize, fmt.c_str(), &bdts);
	if (len <= 0) {
		std::ostringstream msg;
		msg << "Error formatting datetime value string using \"" << fmt << "\" format with strftime(3) system call";
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		return false;
	}
	str.assign(buf, len);
	// Substituting milliseconds if needed
	size_t milliSecondPlaceholderPos = str.find("%f");
	if (milliSecondPlaceholderPos != std::string::npos) {
		std::ostringstream oss;
		oss << std::setfill('0') << std::setw(3) << msec;
		while (milliSecondPlaceholderPos != std::string::npos) {
			str.replace(milliSecondPlaceholderPos, 2, oss.str());
			milliSecondPlaceholderPos = str.find("%f");
		}
	}
	return true;
}

} // namespace isl


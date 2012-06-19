#include <isl/Time.hxx>
#include <isl/DateTime.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <iomanip>
#include <sys/time.h>
#include <errno.h>

namespace isl
{

/*------------------------------------------------------------------------------
 * Time
 * ---------------------------------------------------------------------------*/

const char * Time::DefaultFormat = "%H:%M:%S";
const wchar_t * Time::DefaultWFormat = L"%H:%M:%S";

Time::Time() :
	_millisecond(NullTime),
	_gmtOffset(0)
{}

Time::Time(int hour, int minute, int second, int millisecond, long int gmtOffset) :
	_millisecond(NullTime),
	_gmtOffset(gmtOffset)
{
	setTime(hour, minute, second, millisecond);
}

std::string Time::toString(const std::string& format) const
{
	if (isNull()) {
		return "[null]";
	}
	std::string result;
	if (!DateTime::bdts2str(toBdts(), msecond(), format, result)) {
		result.assign("[invalid time format]");
	}
	return result;
}

struct tm Time::toBdts() const
{
	struct tm bdts;
	memset(&bdts, 0, sizeof(struct tm));
	bdts.tm_sec = second();
	bdts.tm_min = minute();
	bdts.tm_hour = hour();
	bdts.tm_gmtoff = _gmtOffset;
	return bdts;
}

time_t Time::secondsFromEpoch() const
{
	return _millisecond / 1000;
}

bool Time::setTime(int hour, int minute, int second, int millisecond)
{
	if (!isValid(hour, minute, second, millisecond)) {
		_millisecond = NullTime;
		return false;
	}
	_millisecond = (hour * SecondsPerHour + minute * SecondsPerMinute + second) * 1000 + millisecond;
	return true;
}

Time Time::addMSeconds(int nmseconds) const
{
	if (isNull() || (nmseconds == 0)) {
		return *this;
	}
	Time result;
	if (nmseconds < 0) {
		result._millisecond = _millisecond + (nmseconds % MillisecondsPerDay);
		if (result._millisecond < 0) {
			result._millisecond = MillisecondsPerDay + result._millisecond;
		}
	} else {
		result._millisecond = (_millisecond + nmseconds) % MillisecondsPerDay;
	}
	return result;
}

int Time::msecondsTo(const Time& time) const
{
	if (!isValid() || !time.isValid()) {
		return 0;
	}
	return (time._millisecond - _millisecond);
}

int Time::secondsTo(const Time& time) const
{
	if (!isValid() || !time.isValid()) {
		return 0;
	}
	return (time._millisecond - _millisecond) / 1000;
}

void Time::start()
{
	*this = now();
}

int Time::restart()
{
	if (isNull()) {
		return 0;
	}
	Time t = now();
	int n = msecondsTo(t);
	if (n < 0) {
		n += MillisecondsPerDay;
	}
	*this = t;
	return n;
}

int Time::elapsed() const
{
	int n = msecondsTo(now());
	if (n < 0) {
		n += MillisecondsPerDay;
	}
	return n;
}

Time Time::fromSecondsFromEpoch(time_t nsecs, bool isLocalTime)
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
	return Time(bdts.tm_hour, bdts.tm_min, bdts.tm_sec, bdts.tm_gmtoff);
}

Time Time::now()
{
	struct timeval tv;
	struct timezone tz;
	if (gettimeofday(&tv, &tz) != 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::GetTimeOfDay, errno, "Fetching time of day error"));
	}
	tm bdts;
	if (localtime_r(&(tv.tv_sec), &bdts) == NULL) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::LocalTimeR, errno, "Error converting time_t to local time"));
	}
	return Time(bdts.tm_hour, bdts.tm_min, bdts.tm_sec, tv.tv_usec / 1000, bdts.tm_gmtoff);
}

bool Time::operator==(const Time& other) const
{
	if (!isValid() || !other.isValid()) {
		return false;
	}
	return _millisecond == other._millisecond;
}

bool Time::operator!=(const Time& other) const
{
	if (!isValid() || !other.isValid()) {
		return false;
	}
	return _millisecond != other._millisecond;
}

bool Time::operator<(const Time& other) const
{
	if (!isValid() || !other.isValid()) {
		return false;
	}
	return _millisecond < other._millisecond;
}

bool Time::operator<=(const Time& other) const
{
	if (!isValid() || !other.isValid()) {
		return false;
	}
	return _millisecond <= other._millisecond;
}

bool Time::operator>(const Time& other) const
{
	if (!isValid() || !other.isValid()) {
		return false;
	}
	return _millisecond > other._millisecond;
}

bool Time::operator>=(const Time& other) const
{
	if (!isValid() || !other.isValid()) {
		return false;
	}
	return _millisecond >= other._millisecond;
}

bool Time::isValid(int hour, int minute, int second, int millisecond)
{
	return (0 <= hour) && (hour <= 23) && (0 <= minute) && (minute <= 59) && (0 <= second) && (second <= 59) && (0 <= millisecond) &&
		(millisecond <= 999);
	return false;
}

Time Time::fromString(const std::string& str, const std::string& fmt)
{
	struct tm bdts;
	unsigned int msec;
	if (!DateTime::str2bdts(str, fmt, bdts, msec)) {
		return Time();
	}
	return fromBdts(bdts, msec);
}

} // namespace isl

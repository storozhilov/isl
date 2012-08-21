#include <isl/Time.hxx>
#include <isl/DateTime.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <iomanip>
#include <sys/time.h>
#include <errno.h>

namespace isl
{

const char * Time::DefaultFormat = "%H:%M:%S.%f";

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

std::string Time::toString(const std::string& format) const
{
	if (isNull()) {
		return "[null time]";
	}
	std::string result;
	if (!DateTime::bdts2str(toBdts(), _nanoSecond, format, result)) {
		result.assign("[invalid time format]");
	}
	return result;
}

bool Time::set(int hour, int minute, int second, int nanoSecond, long int gmtOffset)
{
	if (!isValid(hour, minute, second, nanoSecond, gmtOffset)) {
		reset();
		return false;
	}
	_isNull = false;
	_second = hour * SecondsPerHour + minute * SecondsPerMinute + second;
	_nanoSecond = nanoSecond;
	_gmtOffset = gmtOffset;
	return true;
}

bool Time::set(time_t secondsFromEpoch, bool isLocalTime, int nanoSecond)
{
	tm bdts;
	if (isLocalTime) {
		if (localtime_r(&secondsFromEpoch, &bdts) == NULL) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::LocalTimeR, errno, "Error converting time_t to local time"));
		}
	} else {
		if (gmtime_r(&secondsFromEpoch, &bdts) == NULL) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::GMTimeR, errno, "Error converting time_t to GMT"));
		}
	}
	return set(bdts, nanoSecond);
}

bool Time::set(const std::string& str, const std::string& fmt)
{
	struct tm bdts;
	int nanoSecond;
	if (DateTime::str2bdts(str, fmt, bdts, nanoSecond)) {
		return set(bdts, nanoSecond);
	} else {
		reset();
		return false;
	}
}

Time Time::addSeconds(long int nSeconds) const
{
	if (isNull() || nSeconds == 0) {
		return *this;
	}
	Time result(*this);
	result._second = (result._second + nSeconds) % SecondsPerDay;
	if (result._second < 0) {
		result._second += SecondsPerDay;
	}
	return result;
}

Time Time::addNanoSeconds(long int nNanoSeconds) const
{
	if (isNull() || nNanoSeconds == 0) {
		return *this;
	}
	Time result(*this);
	result._second = (result._second + (result._nanoSecond + nNanoSeconds) / 1000000000) % SecondsPerDay;
	result._nanoSecond = (result._nanoSecond + nNanoSeconds) % 1000000000;
	if (result._nanoSecond < 0) {
		--result._second;
		result._nanoSecond += 1000000000;
	}
	if (result._second < 0) {
		result._second += SecondsPerDay;
	}
	return result;
}

/*int Time::msecondsTo(const Time& time) const
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
}*/

Time Time::now()
{
	
	timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::ClockGetTime, errno, "Fetching time of day error"));
	}
	tm bdts;
	if (localtime_r(&(ts.tv_sec), &bdts) == NULL) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::LocalTimeR, errno, "Error converting time_t to local time"));
	}
	return Time(bdts.tm_hour, bdts.tm_min, bdts.tm_sec, ts.tv_nsec, bdts.tm_gmtoff);
}

} // namespace isl

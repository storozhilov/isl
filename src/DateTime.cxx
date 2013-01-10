#include <isl/common.hxx>
#include <isl/DateTime.hxx>
#include <isl/TimeSpec.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/LogMessage.hxx>
#include <sys/time.h>
#include <errno.h>
#include <math.h>
#include <iomanip>

//#include <iostream>

namespace isl
{

const char * DateTime::DefaultFormat = "%Y-%m-%d %H:%M:%S.%f";

DateTime::DateTime() :
	BasicDateTime(),
	_isNull(true),
	_ts(TimeSpec::makeZero()),
	_bdts(emptyBdts())
{}

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second, int nanoSec, const TimeZone& tz) :
	BasicDateTime(),
	_isNull(true),
	_ts(TimeSpec::makeZero()),
	_bdts(emptyBdts())
{
	set(year, month, day, hour, minute, second, nanoSec, tz);
}

DateTime::DateTime(const struct tm& bdts, int nanoSec) :
	BasicDateTime(),
	_isNull(true),
	_ts(TimeSpec::makeZero()),
	_bdts(emptyBdts())
{
	set(bdts, nanoSec);
}

DateTime::DateTime(time_t gmtSec, int nanoSec, const TimeZone& tz) :
	BasicDateTime(),
	_isNull(true),
	_ts(TimeSpec::makeZero()),
	_bdts(emptyBdts())
{
	set(gmtSec, nanoSec, tz);
}

DateTime::DateTime(const struct timespec& ts, const TimeZone& tz) :
	BasicDateTime(),
	_isNull(true),
	_ts(TimeSpec::makeZero()),
	_bdts(emptyBdts())
{
	set(ts, tz);
}

DateTime::DateTime(const Timestamp& timestamp, const TimeZone& tz) :
	BasicDateTime(),
	_isNull(true),
	_ts(TimeSpec::makeZero()),
	_bdts(emptyBdts())
{
	set(timestamp, tz);
}

DateTime::DateTime(const std::string& str, const std::string& fmt, const TimeZone& tz) :
	BasicDateTime(),
	_isNull(true),
	_ts(TimeSpec::makeZero()),
	_bdts(emptyBdts())
{
	set(str, fmt, tz);
}

std::string DateTime::toString(const std::string& format) const
{
	if (isNull()) {
		return "[null datetime]";
	}
	std::string result;
	if (!DateTime::bdts2str(_bdts, nanoSecond(), format, result)) {
		result.assign("[invalid datetime format]");
	}
	return result;
}

void DateTime::reset()
{
	_isNull = true;
	TimeSpec::reset(_ts);
	resetBdts(_bdts);
}

bool DateTime::set(int year, int month, int day, int hour, int minute, int second, int nanoSec, const TimeZone& tz)
{
	if (!isValidDate(year, month, day) || !isValidTime(hour, minute, second, nanoSec)) {
		reset();
		return false;
	}
	struct tm bdts = emptyBdts();
	bdts.tm_sec = second;
	bdts.tm_min = minute;
	bdts.tm_hour = hour;
	bdts.tm_mday = day;
	bdts.tm_mon = month - 1;
	bdts.tm_year = year - 1900;
	time_t localSecond = mktime(&bdts);
	if (localSecond < 0) {
		reset();
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::MkTime, errno, "Error converting UNIX breakdown time struct to local time"));
	}
	TimeZone ltz = TimeZone::local();
	time_t gmtSec = localSecond + ltz.gmtOffset() + (ltz.isDst() ? SecondsPerHour : 0) -
		tz.gmtOffset() - (tz.isDst() ? SecondsPerHour : 0);
	//time_t gmtSec = localSecond - ltz.gmtOffset() - (ltz.isDst() ? SecondsPerHour : 0) +
	//	tz.gmtOffset() + (tz.isDst() ? SecondsPerHour : 0);
	return set(gmtSec, nanoSec, tz);
}

bool DateTime::set(time_t gmtSec, int nanoSec, const TimeZone& tz)
{
	if (nanoSec < 0 || nanoSec >= 1000000000) {
		reset();
		return false;
	}
	if (tz == TimeZone::local()) {
		if (!localtime_r(&gmtSec, &_bdts)) {
			reset();
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::LocalTimeR, errno, "Error converting time_t to local time"));
		}
	} else if (tz == TimeZone::gmt()) {
		if (!gmtime_r(&gmtSec, &_bdts)) {
			reset();
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::GMTimeR, errno, "Error converting time_t to GMT time"));
		}
	} else {
		//time_t localSecond = gmtSec - tz.gmtOffset() - (tz.isDst() ? SecondsPerHour : 0);
		time_t localSecond = gmtSec + tz.gmtOffset() + (tz.isDst() ? SecondsPerHour : 0);
		if (!gmtime_r(&localSecond, &_bdts)) {
			reset();
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::GMTimeR, errno, "Error converting time_t to GMT time"));
		}
		tz.apply(_bdts);
	}
	_isNull = false;
	_ts.tv_sec = gmtSec;
	_ts.tv_nsec = nanoSec;
	return true;
}

bool DateTime::set(const std::string& str, const std::string& fmt, const TimeZone& tz)
{
	struct tm bdts;
	int nanoSec;
	if (DateTime::str2bdts(str, fmt, bdts, nanoSec)) {
		tz.apply(bdts);
		return set(bdts, nanoSec);
	} else {
		reset();
		return false;
	}
}

DateTime DateTime::operator+(const Timeout& rhs) const
{
	return isNull() ?
		DateTime() :
		DateTime(gmtSecond() + rhs.seconds() + (nanoSecond() + rhs.nanoSeconds()) / 1000000000L,
				(nanoSecond() + rhs.nanoSeconds()) % 1000000000L, tz());
}

Timeout DateTime::operator-(const DateTime& rhs) const
{
	if (isNull() || rhs.isNull()) {
		return Timeout();
	}
	const DateTime& greaterValue = (*this >= rhs) ? *this : rhs;
	const DateTime& lowerValue = (*this >= rhs) ? rhs : *this;
	time_t timeoutSeconds = greaterValue.gmtSecond() - lowerValue.gmtSecond();
	long int timeoutNanoSeconds = greaterValue.nanoSecond() - lowerValue.nanoSecond();
	if (timeoutNanoSeconds < 0) {
		--timeoutSeconds;
		timeoutNanoSeconds += 1000000000L;
	}
	return Timeout(timeoutSeconds, timeoutNanoSeconds);
}

DateTime DateTime::operator-(const Timeout& rhs) const
{
	time_t gmtSec = gmtSecond() - rhs.seconds();
	long int nanoSec = nanoSecond() - rhs.nanoSeconds();
	if (nanoSec < 0) {
		--gmtSec;
		nanoSec += 1000000000L;
	}
	return DateTime(gmtSec, nanoSec, tz());
}

DateTime& DateTime::operator+=(const Timeout& rhs)
{
	if (!isNull()) {
		set(gmtSecond() + rhs.seconds() + (nanoSecond() + rhs.nanoSeconds()) / 1000000000L,
				(nanoSecond() + rhs.nanoSeconds()) % 1000000000L, tz());
	}
	return *this;
}

DateTime& DateTime::operator-=(const Timeout& rhs)
{
	time_t newGmtSecond = gmtSecond() - rhs.seconds();
	long int newNanoSecond = nanoSecond() - rhs.nanoSeconds();
	if (newNanoSecond < 0) {
		--newGmtSecond;
		newNanoSecond += 1000000000L;
	}
	set(newGmtSecond, newNanoSecond, tz());
	return *this;
}

DateTime DateTime::now(const TimeZone& tz)
{
	timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::ClockGetTime, errno, "Fetching current time error"));
	}
	return DateTime(ts.tv_sec, ts.tv_nsec, tz);
}

/*size_t DateTime::expirationsInFrame(const DateTime& startedFrom, const DateTime& finishedBefore, const Timeout& interval)
{
	if (startedFrom >= finishedBefore || interval.isZero() || (interval >= Timeout(SecondsPerDay))) {
		return 0;
	}
	struct timespec sf = startedFrom.timeSpec();
	sf.tv_sec %= SecondsPerDay;
	struct timespec fb = finishedBefore.timeSpec();
	fb.tv_sec %= SecondsPerDay;
	if (sf >= fb) {
		fb.tv_sec += SecondsPerDay;
	}
	//std::clog << "sf = {" << sf.tv_sec << ", " << sf.tv_nsec << "}, fb = {" << fb.tv_sec << ", " << fb.tv_nsec << "}" << std::endl;
	struct timespec ts = TimeSpec::makeZero();
	size_t expirationsAmount = 0;
	size_t cyclesAmount = 0;
	while (ts < fb) {
		if (ts >= sf) {
			//std::clog << "Expiration found: {" << ts.tv_sec << ", " << ts.tv_nsec << "}" << std::endl;
			++expirationsAmount;
		}
		ts.tv_sec += interval.seconds();
		ts.tv_nsec += interval.nanoSeconds();
		if (ts.tv_nsec > 999999999L) {
			ts.tv_sec += ts.tv_nsec / 1000000000L;
			ts.tv_nsec %= 1000000000L;
		}
		++cyclesAmount;
	}
	//std::clog << "expirationsAmount = " << expirationsAmount << std::endl;
	return expirationsAmount;
}*/

} // namespace isl

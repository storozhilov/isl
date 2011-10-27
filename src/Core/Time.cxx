#include <isl/Time.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <sys/time.h>
#include <errno.h>

namespace isl
{

/*------------------------------------------------------------------------------
 * Time
 * ---------------------------------------------------------------------------*/

const char * Time::IsoInputFormat = "%H:%M:%S";
const wchar_t * Time::IsoInputWFormat = L"%H:%M:%S";
const char * Time::IsoOutputFormat = "%H:%M:%S";
const wchar_t * Time::IsoOutputWFormat = L"%H:%M:%S";

Time::Time() :
	_millisecond(NullTime),
	_gmtOffset(0)
{}

Time::Time(int hour, int minute, int second, int millisecond, int gmtOffset) :
	_millisecond(NullTime),
	_gmtOffset(gmtOffset)
{
	setTime(hour, minute, second, millisecond);
}

bool Time::isNull() const
{
	return (_millisecond == NullTime);
}

bool Time::isValid() const
{
	return !isNull();
}

int Time::hour() const
{
	return (isValid()) ? _millisecond / MillisecondsPerHour : 0;
}

int Time::minute() const
{
	return (isValid()) ? (_millisecond % MillisecondsPerHour) / MillisecondsPerMinute : 0;
}

int Time::second() const
{
	return (isValid()) ? (_millisecond / 1000) % SecondsPerMinute : 0;
}

int Time::msecond() const
{
	return (isValid()) ? _millisecond % 1000 : 0;
}

std::string Time::toString(const std::string& format) const
{
	if (isNull()) {
		return "[null]";
	}
	tm bdt;
	bdt.tm_sec = second();
	bdt.tm_min = minute();
	bdt.tm_hour = hour();
	bdt.tm_mday = 0;
	bdt.tm_mon = 0;
	bdt.tm_year = 0;
	bdt.tm_wday = 0;
	bdt.tm_yday = 0;
	bdt.tm_isdst = 0;
	bdt.tm_gmtoff = _gmtOffset;
	bdt.tm_zone = 0;
	char buf[FormatBufferSize];
	size_t len = strftime(buf, FormatBufferSize, format.c_str(), &bdt);
	if (len <= 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::StrFTime, errno, L"Time formatting error"));
	}
	return std::string(buf, len);
}

std::wstring Time::toWString(const std::wstring& format) const
{
	return String::utf8Decode(toString(String::utf8Encode(format)));
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

void Time::setNull()
{
	_millisecond = NullTime;
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

Time Time::addSeconds(int nseconds) const
{
	return addMSeconds(nseconds * 1000);
}

Time Time::addMinutes(int nminutes) const
{
	return addMSeconds(nminutes * MillisecondsPerMinute);
}

Time Time::addHours(int nhours) const
{
	return addMSeconds(nhours * MillisecondsPerHour);
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
	return Time(bdt.tm_hour, bdt.tm_min, bdt.tm_sec, bdt.tm_gmtoff);
}

Time Time::now()
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
	return Time(bdt.tm_hour, bdt.tm_min, bdt.tm_sec, tv.tv_usec / 1000, bdt.tm_gmtoff);
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
	tm bdt;
	memset(&bdt, 0, sizeof(struct tm));
	// TODO Correct parsing error handling!
	if (strptime(str.c_str(), fmt.c_str(), &bdt) == NULL) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::StrPTime, errno, L"Error parsing time value"));
	}
	return Time(bdt.tm_hour, bdt.tm_min, bdt.tm_sec);
}

Time Time::fromWString(const std::wstring& str, const std::wstring& fmt)
{
	return fromString(String::utf8Encode(str), String::utf8Encode(fmt));
}

} // namespace isl

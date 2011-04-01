#include <isl/Time.hxx>
#include <sstream>
#include <iomanip>
#include <sys/time.h>

namespace isl
{

/*------------------------------------------------------------------------------
 * Time
 * ---------------------------------------------------------------------------*/

Time::Time() :
	_millisecond(0)
{}

Time::Time(int hour, int minute, int second, int millisecond) :
	_millisecond(NullTime)
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

std::wstring Time::toString(const std::wstring& format) const
{
	if (isNull()) {
		return L"null";
	}
	Formatter formatter(*this);
	FormattedWString<Formatter> fs(formatter, &Formatter::substitute, format);
	return fs.str();
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

Time Time::now()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	time_t timeFromEpoch = tv.tv_sec;
	tzset();
	tm localTime;
	localtime_r(&timeFromEpoch, &localTime);
	return Time(localTime.tm_hour, localTime.tm_min, localTime.tm_sec, tv.tv_usec / 1000);
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

/*------------------------------------------------------------------------------
 * Time::Formatter
 * ---------------------------------------------------------------------------*/

Time::Formatter::Formatter(const Time& time) :
	_time(time)
{}

std::wstring Time::Formatter::substitute(wchar_t fmt, const std::wstring& param)
{
	std::wostringstream sstr;
	switch (fmt) {
		case L'h':
			if (param == L"1") {						// Hour with leading zero
				sstr << std::setfill(L'0') << std::setw(2) << _time.hour();
			} else if (param.empty()) {					// Hour
				sstr << _time.hour();
			} else {
				sstr << L"[Unknown format parameters: '" << param << L"']";
			}
			break;
		case L'm':
			if (param == L"1") {						// Minute with leading zero
				sstr << std::setfill(L'0') << std::setw(2) << _time.minute();
			} else if (param.empty()) {					// Minute
				sstr << _time.minute();
			} else {
				sstr << L"[Unknown format parameters: '" << param << L"']";
			}
			break;
		case L's':
			if (param == L"1") {						// Second with leading zero
				sstr << std::setfill(L'0') << std::setw(2) << _time.second();
			} else if (param.empty()) {					// Second
				sstr << _time.second();
			} else {
				sstr << L"[Unknown format parameters: '" << param << L"']";
			}
			break;
		case L'z':
			if (param == L"1") {						// Milliseconds with two leading zeros
				sstr << std::setfill(L'0') << std::setw(3) << _time.msecond();
			} else if (param.empty()) {					// Milliseconds
				sstr << _time.msecond();
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

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

const char * DateTime::DefaultFormat = "%Y-%m-%d %H:%M:%S.%f";

DateTime::DateTime() :
	_date(),
	_time()
{}

DateTime::DateTime(const Date& date, const Time& time) :
	_date(),
	_time()
{
	set(date, time);
}

DateTime::DateTime(time_t secondsFromEpoch, bool isLocalTime, int nanoSecond) :
	_date(),
	_time()
{
	set(secondsFromEpoch, isLocalTime, nanoSecond);
}

DateTime::DateTime(const struct tm& bdts, unsigned int nanoSecond) :
	_date(),
	_time()
{
	set(bdts, nanoSecond);
}

DateTime::DateTime(const std::string& str, const std::string& fmt) :
	_date(),
	_time()
{
	set(str, fmt);
}

bool DateTime::set(const std::string& str, const std::string& fmt)
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

DateTime DateTime::addSeconds(long int nSeconds) const
{
	if (isNull() || (nSeconds == 0)) {
		return *this;
	}
	// TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	return DateTime();
	//return addMSeconds(static_cast<long>(nseconds) * 1000);*/
}

DateTime DateTime::addNanoSeconds(long int nNanoSeconds) const
{
	if (isNull() || (nNanoSeconds == 0)) {
		return *this;
	}
	// TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	return DateTime();
	
	/*Date resultDate(_date);
	Time resultTime(_time);
	resultTime._second += (resultTime._nanoSecond + nNanoSeconds) / 1000000000;
	resultTime._nanoSecond = (resultTime._nanoSecond + nNanoSeconds) % 1000000000;
	if (resultTime._nanoSecond < 0) {
		--resultTime._second;
		resultTime._nanoSecond += 1000000000;
	}
	if (resultTime._second >= Time::SecondsPerDay) {
		resultDate = resultDate.addDays(resultTime._second / Time::SecondsPerDay);
		resultTime._second %= Time::SecondsPerDay;
	} else if (resultTime._second < 0) {
		// TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}
	return DateTime(resultDate, resultTime);*/

	/*int ndays;
	if (nmseconds < 0) {
		ndays = ((nmseconds + _time._nanosecond) < 0) ? (nmseconds + _time._nanosecond) / Time::MillisecondsPerDay - 1 : 0;
	} else {
		ndays = (nmseconds + _time._nanosecond) / Time::MillisecondsPerDay;
	}
	return DateTime(_date.addDays(ndays), _time.addMSeconds(nmseconds));*/
}

std::string DateTime::toString(const std::string& format) const
{
	if (isNull()) {
		return "[null datetime]";
	}
	std::string result;
	if (!DateTime::bdts2str(toBdts(), _time._nanoSecond, format, result)) {
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

bool DateTime::str2bdts(const std::string& str, const std::string& fmt, struct tm& bdts, int& nanoSecond)
{
	memset(&bdts, 0, sizeof(struct tm));
	nanoSecond = 0;
	size_t fmtStartPos = 0;
	const char * strPtr = str.c_str();
	size_t nanoSecondFmtPos;
	do {
		nanoSecondFmtPos = fmt.find("%f", fmtStartPos);
		std::string fmtPart = fmt.substr(fmtStartPos, nanoSecondFmtPos == std::string::npos ? std::string::npos : nanoSecondFmtPos - fmtStartPos);
		strPtr = strptime(strPtr, fmtPart.c_str(), &bdts);
		if (!strPtr) {
			std::ostringstream msg;
			msg << "Error parsing \"" << str << "\" string using \"" << fmt << "\" format with strptime(3) system call";
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			return false;
		}
		if (nanoSecondFmtPos != std::string::npos) {
			nanoSecond = 0;
			for (unsigned int i = 0; i < 9; ++i) {
				if (*strPtr == '\0') {
					// End of the string encountered
					if (i == 0) {
						std::ostringstream msg;
						msg << "Error parsing \"" << str << "\" string using \"" << fmt << "\" format: premature end of nanoseconds value";
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
						return false;
					} else {
						return true;
					}
				} else if (*strPtr < '0' || *strPtr > '9') {
					// Not digit encountered
					if (i == 0) {
						std::ostringstream msg;
						msg << "Error parsing \"" << str << "\" string using \"" << fmt << "\" format: premature end of nanoseconds value";
						debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
						return false;
					} else {
						break;
					}
				}
				nanoSecond = nanoSecond * 10 + *strPtr - '0';
				++strPtr;
			}
			fmtStartPos = nanoSecondFmtPos + 2;
		}
	} while (nanoSecondFmtPos != std::string::npos && fmtStartPos < fmt.length());
	return true;
}

bool DateTime::bdts2str(const struct tm& bdts, int nanoSecond, const std::string& fmt, std::string& str)
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
	// Substituting nanoseconds if needed
	size_t nanoSecondPlaceholderPos = str.find("%f");
	if (nanoSecondPlaceholderPos != std::string::npos) {
		std::ostringstream oss;
		oss << std::setfill('0') << std::setw(9) << nanoSecond;
		while (nanoSecondPlaceholderPos != std::string::npos) {
			str.replace(nanoSecondPlaceholderPos, 2, oss.str());
			nanoSecondPlaceholderPos = str.find("%f");
		}
	}
	return true;
}

} // namespace isl


#include <isl/BasicDateTime.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <string.h>
#include <stdlib.h>

namespace isl
{

const int BasicDateTime::_monthDays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

struct tm BasicDateTime::emptyBdts()
{
	struct tm bdts;
	resetBdts(bdts);
	return bdts;
}

void BasicDateTime::resetBdts(struct tm& bdts)
{
	memset(&bdts, 0, sizeof(bdts));
}

bool BasicDateTime::isLeapYear(int year)
{
	if (year < 1582) {
		return (abs(year) % 4 == 0);
	} else {
		return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
	}
}
bool BasicDateTime::isValidDate(int year, int month, int day)
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

bool BasicDateTime::str2bdts(const std::string& str, const std::string& fmt, struct tm& bdts, int& nanoSecond)
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
			Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Parsing \"") << str << "\" string using \""
					<< fmt << "\" format error with strptime(3) system call");
			return false;
		}
		if (nanoSecondFmtPos != std::string::npos) {
			nanoSecond = 0;
			for (unsigned int i = 0; i < 9; ++i) {
				if (*strPtr == '\0') {
					// End of the string encountered
					if (i == 0) {
						Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Parsing \"") << str << "\" string using \"" <<
								fmt << "\" format error: premature end of nanoseconds value");
						return false;
					} else {
						return true;
					}
				} else if (*strPtr < '0' || *strPtr > '9') {
					// Not digit encountered
					if (i == 0) {
						Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Parsing \"") << str << "\" string using \"" <<
								fmt << "\" format error: premature end of nanoseconds value");
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

bool BasicDateTime::bdts2str(const struct tm& bdts, int nanoSecond, const std::string& fmt, std::string& str)
{
	str.clear();
	char buf[FormatBufferSize];
	size_t len = strftime(buf, FormatBufferSize, fmt.c_str(), &bdts);
	if (len <= 0) {
		Log::debug().log(LogMessage(SOURCE_LOCATION_ARGS, "Formatting datetime value string using \"") <<
				fmt << "\" format error with strftime(3) system call");
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

#ifndef ISL__ABSTRACT_DATE_TIME_FORMATTER__HXX
#define ISL__ABSTRACT_DATE_TIME_FORMATTER__HXX

#include <isl/AbstractFormatter.hxx>
#include <isl/String.hxx>

namespace isl
{

/*
   These classes has been developed for future use only. It's purpose is for extending datetime format in a future.
   TODO MOve it to the experimental staff.
*/

/*const char * Date::monthNames[] = { "", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
const char * Date::monthGMTNames[] = { "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
const char * Date::daysOfWeekNames[] = { "", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };
const char * Date::daysOfWeekGMTNames[] = { "", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };*/

//! Base class for date and time formatting
/*!
    TODO Modifiers support: see date(1)
*/
template <typename Ch> class AbstractDateTimeFormatter : public AbstractFormatter<Ch>
{
public:
	AbstractDateTimeFormatter(const std::basic_string<Ch>& format, Ch tokenSpecifier = '%') :
		AbstractFormatter<Ch>(format),
		_tokenSpecifier(tokenSpecifier)
	{}
protected:
	virtual std::basic_string<Ch> substitute(Ch tokenSymbol) const = 0;
private:
	AbstractDateTimeFormatter();

	virtual typename isl::AbstractFormatter<Ch>::TokenPosition findToken(size_t startPosition = 0) const
	{
		typename isl::AbstractFormatter<Ch>::TokenPosition result(startPosition, 0);
		while (true) {
			if (result.first >= (isl::AbstractFormatter<Ch>::_format.length() - 1)) {
				result.first = std::basic_string<Ch>::npos;
				return result;
			}
			// Finding format specifier:
			result.first = isl::AbstractFormatter<Ch>::_format.find(_tokenSpecifier, result.first);
			if (result.first == std::basic_string<Ch>::npos) {
				return result;
			}
			if (result.first >= (isl::AbstractFormatter<Ch>::_format.length() - 1)) {
				result.first = std::basic_string<Ch>::npos;
				return result;
			}
			// Parsing token
			result.second = 2;
			if ((isl::AbstractFormatter<Ch>::_format[result.first + result.second - 1] == _tokenSpecifier) ||
					String::isChar(isl::AbstractFormatter<Ch>::_format[result.first + result.second - 1])) {
				// Token found -> return its positions
				return result;
			} else {
				// Not a token -> lookup further
				++result.first;
			}
		}
	}
	virtual std::basic_string<Ch> substituteToken(const std::basic_string<Ch>& token) const
	{
		if (token[1] == _tokenSpecifier) {
			// Returning "<token_specifier>" if "<token_specifier><token_specifier>" token has been provided
			return std::basic_string<Ch>(1, _tokenSpecifier);
		}
		return substitute(token[1]);
	}

	Ch _tokenSpecifier;
};

/*------------------------------------------------------------------------------
 * BasicTimeFormatter
 * TODO Modifiers support: see date(1)
 * ---------------------------------------------------------------------------*/

/*template <typename Ch> class BasicTimeFormatter : public AbstractDateTimeFormatter<Ch>
{
public:
	BasicTimeFormatter(const std::basic_string<Ch>& format, const Time& time, Ch tokenSpecifier = '%') :
		AbstractDateTimeFormatter<Ch>(format, tokenSpecifier),
		_time(time)
	{}
private:
	BasicTimeFormatter();

	virtual std::basic_string<Ch> substitute(Ch tokenSymbol) const;

	const Time& _time;
};

typedef BasicTimeFormatter<char> TimeFormatter;
typedef BasicTimeFormatter<wchar_t> WTimeFormatter;

template <> std::string TimeFormatter::substitute(char tokenSymbol) const
{
	std::ostringstream oss;
	switch (tokenSymbol) {
		case 'H':
			// hour (00..23)
			oss << std::setfill('0') << std::setw(2) << _time.hour();
			break;
		case 'I':
			// hour (01..12)
			oss << std::setfill('0') << std::setw(2) << ((_time.hour() > 12) ? _time.hour() - 12 : _time.hour());
			break;
		case 'k':
			// hour ( 0..23)
			oss << _time.hour();
			break;
		case 'l':
			// hour ( 1..12)
			oss << ((_time.hour() > 12) ? _time.hour() - 12 : _time.hour());
			break;
		case 'M':
			// minute (00..59)
			oss << std::setfill('0') << std::setw(2) << _time.minute();
			break;
		case 'n':
			// a newline
			oss << '\n';
		case 'N':
			// nanoseconds (000000000..999999999)
			// TODO
			oss << "['" << tokenSymbol << "' token is not implemented yet]";
			break;
		case 'p':
			// locale's equivalent of either AM or PM; blank if not known
			// TODO
			oss << "['" << tokenSymbol << "' token is not implemented yet]";
			break;
		case 'P':
			// like %p, but lower case
			// TODO
			oss << "['" << tokenSymbol << "' token is not implemented yet]";
			break;
		case 'r':
			// locale's 12-hour clock time (e.g., 11:11:04 PM)
			// TODO
			oss << "['" << tokenSymbol << "' token is not implemented yet]";
			break;
		case 'R':
			// 24-hour hour and minute; same as %H:%M
			oss << std::setfill('0') << std::setw(2) << _time.hour() << ':' << std::setw(2) << _time.minute();
			break;
		case 's':
			// seconds since 1970-01-01 00:00:00 UTC
			// TODO
			oss << "['" << tokenSymbol << "' token is not implemented yet]";
			break;
		case 'S':
			// second (00..60)
			oss << std::setfill('0') << std::setw(2) << _time.second();
			break;
		case 't':
			// a tab
			oss << '\t';
		case 'T':
			// time; same as %H:%M:%S
			oss << std::setfill('0') << std::setw(2) << _time.hour() << ':' <<
				std::setw(2) << _time.minute() << ':' << std::setw(2) << _time.second();
			break;
		default:
			oss << "[Invalid token: '" << tokenSymbol << "']";
	}
	return oss.str();
}

template <> std::wstring WTimeFormatter::substitute(wchar_t tokenSymbol) const
{
	std::wostringstream oss;
	switch (tokenSymbol) {
		case L'H':
			// hour (00..23)
			oss << std::setfill(L'0') << std::setw(2) << _time.hour();
			break;
		case L'I':
			// hour (01..12)
			oss << std::setfill(L'0') << std::setw(2) << ((_time.hour() > 12) ? _time.hour() - 12 : _time.hour());
			break;
		case L'k':
			// hour ( 0..23)
			oss << _time.hour();
			break;
		case L'l':
			// hour ( 1..12)
			oss << ((_time.hour() > 12) ? _time.hour() - 12 : _time.hour());
			break;
		case L'M':
			// minute (00..59)
			oss << std::setfill(L'0') << std::setw(2) << _time.minute();
			break;
		case L'n':
			// a newline
			oss << L'\n';
		case L'N':
			// nanoseconds (000000000..999999999)
			// TODO
			oss << L"['" << tokenSymbol << L"' token is not implemented yet]";
			break;
		case L'p':
			// locale's equivalent of either AM or PM; blank if not known
			// TODO
			oss << L"['" << tokenSymbol << L"' token is not implemented yet]";
			break;
		case L'P':
			// like %p, but lower case
			// TODO
			oss << L"['" << tokenSymbol << L"' token is not implemented yet]";
			break;
		case L'r':
			// locale's 12-hour clock time (e.g., 11:11:04 PM)
			// TODO
			oss << L"['" << tokenSymbol << L"' token is not implemented yet]";
			break;
		case L'R':
			// 24-hour hour and minute; same as %H:%M
			oss << std::setfill(L'0') << std::setw(2) << _time.hour() << ':' << std::setw(2) << _time.minute();
			break;
		case L's':
			// seconds since 1970-01-01 00:00:00 UTC
			// TODO
			oss << L"['" << tokenSymbol << L"' token is not implemented yet]";
			break;
		case L'S':
			// second (00..60)
			oss << std::setfill(L'0') << std::setw(2) << _time.second();
			break;
		case L't':
			// a tab
			oss << L'\t';
		case L'T':
			// time; same as %H:%M:%S
			oss << std::setfill(L'0') << std::setw(2) << _time.hour() << L':' <<
				std::setw(2) << _time.minute() << L':' << std::setw(2) << _time.second();
			break;
		default:
			oss << L"[Invalid token: '" << tokenSymbol << L"']";
	}
	return oss.str();
}*/

/*------------------------------------------------------------------------------
 * BasicDateFormatter
 * TODO Modifiers support: see date(1)
 * ---------------------------------------------------------------------------*/

/*template <typename Ch> class BasicDateFormatter : public AbstractDateTimeFormatter<Ch>
{
public:
	BasicDateFormatter(const std::basic_string<Ch>& format, const Date& date, Ch tokenSpecifier = '%') :
		AbstractDateTimeFormatter<Ch>(format, tokenSpecifier),
		_date(date)
	{}
private:
	BasicDateFormatter();

	virtual std::basic_string<Ch> substitute(Ch tokenSymbol) const;

	const Date& _date;
};

typedef BasicDateFormatter<char> DateFormatter;
typedef BasicDateFormatter<wchar_t> WDateFormatter;

template <> std::string DateFormatter::substitute(char tokenSymbol) const
{
	std::ostringstream oss;
	switch (tokenSymbol) {
		case 'a':
			// locale's abbreviated weekday name (e.g., Sun)
			// TODO Locale support
			oss << Date::daysOfWeekGMTNames[_date.dayOfWeek()];
			break;
		case 'A':
			// locale's full weekday name (e.g., Sunday)
			// TODO Locale support
			oss << Date::daysOfWeekNames[_date.dayOfWeek()];
			break;
		case 'b':
			// locale's abbreviated month name (e.g., Jan)
			// TODO Locale support
			oss << Date::monthGMTNames[_date.month()];
			break;
		case 'B':
			// locale's full month name (e.g., January)
			// TODO Locale support
			oss << Date::monthNames[_date.month()];
			break;
		case 'C':
			// century; like %Y, except omit last two digits (e.g., 20)
			oss << ((int) _date.year() / 100);
			break;
		case 'd':
			// day of month (e.g, 01)
			oss << std::setfill('0') << std::setw(2) << _date.day();
			break;
		case 'D':
			// date; same as %m/%d/%y
			{
				double dummy;
				oss << std::setfill('0') << std::setw(2) << _date.month() << '/' << std::setw(2) << _date.day() <<
					'/' << std::setw(2) << modf(_date.year() / 100.0, &dummy) * 100;
			}
			break;
		case 'e':
			// day of month, space padded; same as %_d
			oss << std::setfill(' ') << std::setw(2) << _date.day();
			break;
		case 'F':
			// full date; same as %Y-%m-%d
			oss << std::setfill('0') << std::setw(4) << _date.year() << '-' << std::setw(2) << _date.month() << '-' << std::setw(2) << _date.day();
			break;
		case 'g':
			// last two digits of year of ISO week number (see %G)
			{
				int weekNumberYear;
				double dummy;
				_date.weekNumber(weekNumberYear);
				oss << std::setfill('0') << std::setw(2) << modf(weekNumberYear / 100.0, &dummy) * 100;
			}
			break;
		case 'G':
			// year of ISO week number (see %V); normally useful only with %V
			{
				int weekNumberYear;
				_date.weekNumber(weekNumberYear);
				oss << weekNumberYear;
			}
			break;
		case 'h':
			// same as %b
			// TODO Locale support
			oss << Date::monthGMTNames[_date.month()];
			break;
		case 'j':
			// day of year (001..366)
			oss << std::setfill('0') << std::setw(3) << _date.dayOfYear();
			break;
		case 'm':
			// month (01..12)
			oss << std::setfill('0') << std::setw(2) << _date.month();
			break;
		case 'n':
			// a newline
			oss << '\n';
			break;
		case 't':
			// a tab
			oss << '\t';
			break;
		case 'u':
			// day of week (1..7); 1 is Monday
			oss << _date.dayOfWeek();
			break;
		case 'U':
			// week number of year, with Sunday as first day of week (00..53)
			oss << std::setfill('0') << std::setw(2) << _date.weekNumber(false);
			break;
		case 'V':
			// ISO week number, with Monday as first day of week (01..53)
			oss << std::setfill('0') << std::setw(2) << _date.weekNumber();
			break;
		case 'w':
			// day of week (0..6); 0 is Sunday
			oss << (_date.dayOfWeek(false) - 1);
			break;
		case 'W':
			// week number of year, with Monday as first day of week (00..53)
			oss << std::setfill('0') << std::setw(2) << _date.weekNumber();
			break;
		case 'x':
			// locale's date representation (e.g., 12/31/99)
			// TODO
			oss << "['" << tokenSymbol << "' token is not implemented yet]";
			break;
		case 'y':
			// last two digits of year (00..99)
			{
				double dummy;
				oss << std::setfill('0') << std::setw(2) << modf(_date.year() / 100.0, &dummy) * 100;
			}
			break;
		case 'Y':
			// year
			oss << _date.year();
			break;
		default:
			oss << "[Invalid token: '" << tokenSymbol << "']";
	}
	return oss.str();
}

template <> std::wstring WDateFormatter::substitute(wchar_t tokenSymbol) const
{
	std::wostringstream oss;
	switch (tokenSymbol) {
		case L'a':
			// locale's abbreviated weekday name (e.g., Sun)
			// TODO Locale support
			oss << Date::daysOfWeekGMTNames[_date.dayOfWeek()];
			break;
		case L'A':
			// locale's full weekday name (e.g., Sunday)
			// TODO Locale support
			oss << Date::daysOfWeekNames[_date.dayOfWeek()];
			break;
		case L'b':
			// locale's abbreviated month name (e.g., Jan)
			// TODO Locale support
			oss << Date::monthGMTNames[_date.month()];
			break;
		case L'B':
			// locale's full month name (e.g., January)
			// TODO Locale support
			oss << Date::monthNames[_date.month()];
			break;
		case L'C':
			// century; like %Y, except omit last two digits (e.g., 20)
			oss << ((int) _date.year() / 100);
			break;
		case L'd':
			// day of month (e.g, 01)
			oss << std::setfill(L'0') << std::setw(2) << _date.day();
			break;
		case L'D':
			// date; same as %m/%d/%y
			{
				double dummy;
				oss << std::setfill(L'0') << std::setw(2) << _date.month() << L'/' << std::setw(2) << _date.day() <<
					L'/' << std::setw(2) << modf(_date.year() / 100.0, &dummy) * 100;
			}
			break;
		case L'e':
			// day of month, space padded; same as %_d
			oss << std::setfill(L' ') << std::setw(2) << _date.day();
			break;
		case L'F':
			// full date; same as %Y-%m-%d
			oss << std::setfill(L'0') << std::setw(4) << _date.year() << L'-' << std::setw(2) << _date.month() << L'-' << std::setw(2) << _date.day();
			break;
		case L'g':
			// last two digits of year of ISO week number (see %G)
			{
				int weekNumberYear;
				double dummy;
				_date.weekNumber(weekNumberYear);
				oss << std::setfill(L'0') << std::setw(2) << modf(weekNumberYear / 100.0, &dummy) * 100;
			}
			break;
		case L'G':
			// year of ISO week number (see %V); normally useful only with %V
			{
				int weekNumberYear;
				_date.weekNumber(weekNumberYear);
				oss << weekNumberYear;
			}
			break;
		case L'h':
			// same as %b
			// TODO Locale support
			oss << Date::monthGMTNames[_date.month()];
			break;
		case L'j':
			// day of year (001..366)
			oss << std::setfill(L'0') << std::setw(3) << _date.dayOfYear();
			break;
		case L'm':
			// month (01..12)
			oss << std::setfill(L'0') << std::setw(2) << _date.month();
			break;
		case L'n':
			// a newline
			oss << L'\n';
			break;
		case L't':
			// a tab
			oss << L'\t';
			break;
		case L'u':
			// day of week (1..7); 1 is Monday
			oss << _date.dayOfWeek();
			break;
		case L'U':
			// week number of year, with Sunday as first day of week (00..53)
			oss << std::setfill(L'0') << std::setw(2) << _date.weekNumber(false);
			break;
		case L'V':
			// ISO week number, with Monday as first day of week (01..53)
			oss << std::setfill(L'0') << std::setw(2) << _date.weekNumber();
			break;
		case L'w':
			// day of week (0..6); 0 is Sunday
			oss << (_date.dayOfWeek(false) - 1);
			break;
		case L'W':
			// week number of year, with Monday as first day of week (00..53)
			oss << std::setfill(L'0') << std::setw(2) << _date.weekNumber();
			break;
		case L'x':
			// locale's date representation (e.g., 12/31/99)
			// TODO
			oss << L"['" << tokenSymbol << L"' token is not implemented yet]";
			break;
		case L'y':
			// last two digits of year (00..99)
			{
				double dummy;
				oss << std::setfill(L'0') << std::setw(2) << modf(_date.year() / 100.0, &dummy) * 100;
			}
			break;
		case 'Y':
			// year
			oss << _date.year();
			break;
		default:
			oss << L"[Invalid token: '" << tokenSymbol << L"']";
	}
	return oss.str();
}*/

} // namespace isl

#endif

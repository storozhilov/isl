#ifndef ISL__DATE__HXX
#define ISL__DATE__HXX

#include <isl/FormattedString.hxx>
#include <string>

namespace isl
{

class Date
{
public:
	Date();
	Date(int year, int month, int day);

	bool isNull() const;
	bool isValid() const;
	int day() const;
	int dayOfWeek() const;
	int dayOfYear() const;
	int weekNumber() const;
	int weekNumber(int& actualYear) const;
	int daysInMonth() const;
	int daysInYear() const;
	int daysTo(const Date& date) const;
	int month() const;
	int year() const;
	bool setDate(int year, int month, int day);
	void setNull();
	Date addDays(int ndays) const;
	Date addMonths(int nmonths) const;
	Date addYears(int nyears) const;
	std::wstring toString(const std::wstring& format) const;
	bool operator==(const Date& other) const;
	bool operator!=(const Date& other) const;
	bool operator<(const Date& other) const;
	bool operator<=(const Date& other) const;
	bool operator>(const Date& other) const;
	bool operator>=(const Date& other) const;

	const static char * monthGMTNames[];
	const static char * daysOfWeekGMTNames[];

	static bool isLeapYear(int year);
	static bool isValid(int year, int month, int day);
	static int daysInYear(int year);
	static int daysInMonth(int year, int month);
	static Date now();
private:
	class Formatter
	{
	public:
		Formatter(const Date& date);

		std::wstring substitute(wchar_t fmt, const std::wstring& param = std::wstring());
	private:
		Formatter();

		const Date& _date;
	};
	
	const static int _monthDays[];

	static int dayNumberFromDate(int year, int month, int day);
	static void dateFromDayNumber(int dayNumber, int& year, int& month, int& day);

	int _dayNumber;
	int _year;
	int _month;
	int _day;

	friend class FormattedWString<Date>;
	friend class DateTime;
};

} // namespace isl

#endif


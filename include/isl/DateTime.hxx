#ifndef ISL__DATE_TIME__HXX
#define ISL__DATE_TIME__HXX

#include <isl/Date.hxx>
#include <isl/Time.hxx>

#include <time.h>
#include <string>

namespace isl
{

class DateTime
{
public:
	DateTime();
	DateTime(const Date& date);
	DateTime(const Date& date, const Time& time);
	DateTime(time_t t);							// TODO This method does not work properly (maybe something in TZ)!

	bool isNull() const;
	bool isValid() const;
	Date date() const;
	Time time() const;
	bool setDate(const Date& d);
	bool setTime(const Time& t);
	void setNull();
	time_t toTime_t() const;						// TODO This method does not work properly (maybe something in TZ)!
	void setTime_t(time_t t);						// TODO This method does not work properly (maybe something in TZ)!
	DateTime addDays(int ndays) const;
	DateTime addMonths(int nmonths) const;
	DateTime addYears(int nyears) const;
	DateTime addMSeconds(long nmseconds) const;
	DateTime addSeconds(int nseconds) const;
	DateTime addMinutes(int nminutes) const;
	DateTime addHours(int nhours) const;
	std::wstring toString(const std::wstring& format) const;
	std::wstring toGMT() const;
	bool operator==(const DateTime& other) const;
	bool operator!=(const DateTime& other) const;
	bool operator<(const DateTime& other) const;
	bool operator<=(const DateTime& other) const;
	bool operator>(const DateTime& other) const;
	bool operator>=(const DateTime& other) const;
	
	static DateTime now();
private:
	class Formatter
	{
	public:
		Formatter(const DateTime& datetime);

		std::wstring substitute(wchar_t fmt, const std::wstring& param = std::wstring());
	private:
		Formatter();

		const DateTime& _datetime;
	};

	Date _date;
	Time _time;

	friend class FormattedString<DateTime>;
	friend class FormattedWString<DateTime>;
};

} // namespace isl

#endif


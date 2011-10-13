#ifndef ISL__TIME__HXX
#define ISL__TIME__HXX

#include <string>
#include <time.h>

namespace isl
{

class Time
{
public:
	Time();
	Time(int hour, int minute, int second = 0, int millisecond = 0);

	bool isNull() const;
	bool isValid() const;
	int hour() const;
	int minute() const;
	int second() const;
	int msecond() const;
	std::string toString(const std::string& format) const;
	std::wstring toWString(const std::wstring& format) const;
	bool setTime(int hour, int minute, int second, int millisecond = 0);
	void setNull();
	Time addMSeconds(int nmseconds) const;
	Time addSeconds(int nseconds) const;
	Time addMinutes(int nminutes) const;
	Time addHours(int nhours) const;
	int msecondsTo(const Time& time) const;
	int secondsTo(const Time& time) const;
	void start();
	int restart();
	int elapsed() const;
	bool operator==(const Time& other) const;
	bool operator!=(const Time& other) const;
	bool operator<(const Time& other) const;
	bool operator<=(const Time& other) const;
	bool operator>(const Time& other) const;
	bool operator>=(const Time& other) const;

	static Time now();
	static bool isValid(int hour, int minute, int second, int millisecond = 0);

private:
	enum Consts {
		NullTime = -1,
		SecondsPerDay = 86400,
		MillisecondsPerDay = 86400000,
		SecondsPerHour = 3600,
		MillisecondsPerHour = 3600000,
		SecondsPerMinute = 60,
		MillisecondsPerMinute = 60000
	};

	class Formatter
	{
	public:
		Formatter(const Time& time);

		std::wstring substitute(wchar_t fmt, const std::wstring& param = std::wstring());
	private:
		Formatter();

		const Time& _time;
	};

	int _millisecond;
	
	friend class DateTime;
};

} // namespace isl

#endif


#ifndef ISL__DATE_TIME__HXX
#define ISL__DATE_TIME__HXX

#include <isl/Date.hxx>
#include <isl/Time.hxx>

namespace isl
{

//! Nanosecond-precision datetime
class DateTime
{
public:
	//! Default datetime format
	static const char * DefaultFormat;

	//! Constructs NULL datetime
	DateTime();
	//! Constructs datetime from the date & time values
	DateTime(const Date& date, const Time& time);
	//! Constructs datetime from the time_t value
	/*!
	  \param secondsFromEpoch Seconds from the Epoch (1970-01-01)
	  \param isLocalTime Does the time_t value should be treated as local time or GMT one
	  \param nanoSecond Nanoseconds
	*/
	DateTime(time_t secondsFromEpoch, bool isLocalTime, int nanoSecond = 0);
	//! Constructs datetime from the UNIX break-down time structure
	/*!
	  \param bdts UNIX break-down time structure
	  \param nanoSecond Nanoseconds
	*/
	DateTime(const struct tm& bdts, unsigned int nanoSecond = 0);
	//! Constructs datetime from the string using supplied format
	/*!
	    \param str String to parse
	    \param fmt Datetime format (see man strftime) including '%f' for nanoseconds
	*/
	DateTime(const std::string& str, const std::string& fmt = std::string(DefaultFormat));

	//! Inspects for NULL time
	inline bool isNull() const
	{
		return _date.isNull() || _time.isNull();
	}
	//! Returns date part of the datetime
	inline const Date& date() const
	{
		return _date;
	}
	//! Returns time part of the datetime
	inline const Time& time() const
	{
		return _time;
	}
	//! Sets date part of the datetime
	inline void setDate(const Date& d)
	{
		_date = d;
		if (_date.isNull() || _time.isNull()) {
			reset();
		}
	}
	//! Sets time part of the datetime
	inline void setTime(const Time& t)
	{
		_time = t;
		if (_date.isNull() || _time.isNull()) {
			reset();
		}
	}
	//! Sets datetime value
	/*!
	  \param d Date
	  \param t Time
	  \return TRUE if the new datetime value is not NULL datetime
	*/
	inline bool set(const Date& d, const Time& t)
	{
		_date = d;
		_time = t;
		if (_date.isNull() || _time.isNull()) {
			reset();
		}
		return !isNull();
	}
	//! Sets datetime from the time_t value
	/*!
	  \param secondsFromEpoch Seconds from the Epoch (1970-01-01)
	  \param isLocalTime Does the time_t value should be treated as local time or GMT one
	  \param nanoSecond Nanoseconds
	  \return TRUE if the new datetime value is not NULL datetime
	*/
	inline bool set(time_t secondsFromEpoch, bool isLocalTime, int nanoSecond = 0)
	{
		return set(Date(secondsFromEpoch, isLocalTime), Time(secondsFromEpoch, isLocalTime, nanoSecond));
	}
	//! Sets datetime value from the UNIX break-down time structure
	/*!
	  \param bdts UNIX break-down time structure
	  \param nanoSecond Nanoseconds
	  \return TRUE if the new datetime value is not NULL datetime
	*/
	inline bool set(const struct tm& bdts, unsigned int nanoSecond = 0)
	{
		return set(Date(bdts), Time(bdts, nanoSecond));
	}
	//! Sets datetime from the string using supplied format
	/*!
	  \param str String to parse
	  \param fmt Datetime format (see man strftime) including '%f' for nanoseconds
	  \return TRUE if the new datetime value is not NULL datetime
	*/
	bool set(const std::string& str, const std::string& fmt = std::string(DefaultFormat));
	//! Sets NULL datetime
	inline void reset()
	{
		_date.reset();
		_time.reset();
	}
	//! Returns copy of the object with added passed number of years
	/*!
	    \param nYears Number of years to add to the date
	*/
	inline DateTime addYears(int nYears) const
	{
		if (isNull() || (nYears == 0)) {
			return *this;
		}
		return DateTime(_date.addYears(nYears), _time);
	}
	//! Returns copy of the object with added passed number of months
	/*!
	    \param nMonths Number of months to add to the date
	*/
	inline DateTime addMonths(int nMonths) const
	{
		if (isNull() || (nMonths == 0)) {
			return *this;
		}
		return DateTime(_date.addMonths(nMonths), _time);
	}
	//! Returns copy of the object with added passed number of days
	/*!
	    \param nDays Number of day to add to the date
	*/
	inline DateTime addDays(int nDays) const
	{
		if (isNull() || (nDays == 0)) {
			return *this;
		}
		return DateTime(_date.addDays(nDays), _time);
	}
	//! Returns copy of the object with added passed number of hours
	inline DateTime addHours(int nHours) const
	{
		return addSeconds(static_cast<long>(nHours) * Time::SecondsPerHour);
	}
	//! Returns copy of the object with added passed number of minutes
	inline DateTime addMinutes(int nMinutes) const
	{
		return addSeconds(static_cast<long int>(nMinutes) * Time::SecondsPerMinute);
	}
	//! Returns copy of the object with added passed number of seconds
	DateTime addSeconds(long int nSeconds) const;
	//! Returns copy of the object with added passed number of nanoseconds
	DateTime addNanoSeconds(long int nNanoSeconds) const;
	//! Formats datetime value with given format
	/*!
	    \param format Datetime format (see man strftime) including '%f' for nanoseconds
	    \return Formatted datetime value
	*/
	std::string toString(const std::string& format = std::string(DefaultFormat)) const;
	//! Converts datetime value to UNIX break-down time structure
	struct tm toBdts() const;
	//! Returns seconds since Epoch
	inline time_t toSecondsFromEpoch() const
	{
		return _date.toSecondsFromEpoch() + _time.toSecondsFromEpoch();
	}
	//! Comparence operator
	/*!
	  \param rhs Another datetime value to compare with
	*/
	inline bool operator==(const DateTime& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return isEqualsTo(rhs);
	}
	//! Comparence operator
	/*!
	  \param rhs Another datetime value to compare with
	*/
	inline bool operator!=(const DateTime& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return !isEqualsTo(rhs);
	}
	//! Comparence operator
	/*!
	  \param rhs Another datetime value to compare with
	*/
	inline bool operator<(const DateTime& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return isLessThen(rhs);
	}
	//! Comparence operator
	/*!
	  \param rhs Another datetime value to compare with
	*/
	inline bool operator<=(const DateTime& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return !rhs.isLessThen(*this);
	}
	//! Comparence operator
	/*!
	  \param rhs Another datetime value to compare with
	*/
	inline bool operator>(const DateTime& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return rhs.isLessThen(*this);
	}
	//! Comparence operator
	/*!
	  \param rhs Another datetime value to compare with
	*/
	inline bool operator>=(const DateTime& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return !isLessThen(rhs);
	}
	
	//! Returns current datetime value for local time
	inline static DateTime now()
	{
		return DateTime(Date::now(), Time::now());
	}
private:
	enum Consts {
		FormatBufferSize = 4096
	};

	inline bool isEqualsTo(const DateTime& rhs) const
	{
		return ((_date == rhs._date) && (_time == rhs._time));
	}
	inline bool isLessThen(const DateTime& rhs) const
	{
		return _date < rhs._date || (_date == rhs._date && _time < rhs._time);
	}

	static bool str2bdts(const std::string& str, const std::string& fmt, struct tm& bdts, int& nanoSecond);
	static bool bdts2str(const struct tm& bdts, int nanoSecond, const std::string& fmt, std::string& str);

	Date _date;
	Time _time;

	friend class Time;
	friend class Date;
};

} // namespace isl

#endif


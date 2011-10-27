#ifndef ISL__DATE_TIME__HXX
#define ISL__DATE_TIME__HXX

#include <isl/Date.hxx>
#include <isl/Time.hxx>

namespace isl
{

//! Date & time representation class
class DateTime
{
public:
	//! Constructs NULL datetime
	DateTime();
	//! Constructs datetime value from the date value
	DateTime(const Date& date);
	//! Constructs datetime value from the date & time values
	DateTime(const Date& date, const Time& time);

	//! Inspects for NULL time
	bool isNull() const;
	//! Opposite of isNull()
	bool isValid() const;
	//! Returns date part of the datetime
	Date date() const;
	//! Returns time part of the datetime
	Time time() const;
	//! Sets date part of the datetime
	bool setDate(const Date& d);
	//! Sets time part of the datetime
	bool setTime(const Time& t);
	//! Sets NULL datetime
	void setNull();
	//! Returns copy of the object with added passed number of days
	/*!
	    \param ndays Number of day to add to the date
	*/
	DateTime addDays(int ndays) const;
	//! Returns copy of the object with added passed number of months
	/*!
	    \param nmonths Number of months to add to the date
	*/
	DateTime addMonths(int nmonths) const;
	//! Returns copy of the object with added passed number of years
	/*!
	    \param nyears Number of years to add to the date
	*/
	DateTime addYears(int nyears) const;
	//! Adds milliseconds
	DateTime addMSeconds(long nmseconds) const;
	//! Adds seconds
	DateTime addSeconds(int nseconds) const;
	//! Adds minutes
	DateTime addMinutes(int nminutes) const;
	//! Adds hours
	DateTime addHours(int nhours) const;
	//! Formats datetime value to the one-byte character string
	/*!
	    \param format Time format - see date(1)
	    TODO Milliseconds support
	*/
	std::string toString(const std::string& format = std::string(IsoOutputFormat)) const;
	//! Formats datetime value to the wide character string
	/*!
	    \param format Time format - see date(1)
	    TODO Milliseconds support
	*/
	std::wstring toWString(const std::wstring& format = std::wstring(IsoOutputWFormat)) const;
	//! Returns seconds since Epoch
	time_t secondsFromEpoch() const;
	//! Comparence operator
	bool operator==(const DateTime& other) const;
	//! Comparence operator
	bool operator!=(const DateTime& other) const;
	//! Comparence operator
	bool operator<(const DateTime& other) const;
	//! Comparence operator
	bool operator<=(const DateTime& other) const;
	//! Comparence operator
	bool operator>(const DateTime& other) const;
	//! Comparence operator
	bool operator>=(const DateTime& other) const;
	
	//! HTTP datetime narrow character input format string
	static const char * HttpInputFormat;
	//! HTTP datetime wide character input format string
	static const wchar_t * HttpInputWFormat;
	//! HTTP datetime narrow character output format string
	static const char * HttpOutputFormat;
	//! HTTP datetime wide character output format string
	static const wchar_t * HttpOutputWFormat;


	//! ISO datetime narrow character input format string
	static const char * IsoInputFormat;
	//! ISO datetime wide character input format string
	static const wchar_t * IsoInputWFormat;
	//! ISO datetime narrow character output format string
	static const char * IsoOutputFormat;
	//! ISO datetime wide character output format string
	static const wchar_t * IsoOutputWFormat;

	//! Composes new DateTime object from time_t value
	/*!
	    \param nsecs Seconds from the Epoch (1970-01-01)
	    \param isLocalTime 'true' or 'false' if the time_t value should be treated as local time or GMT
	    \return new Date object
	*/
	static DateTime fromSecondsFromEpoch(time_t nsecs, bool isLocalTime);
	//! Returns current datetime value for local time
	static DateTime now();
	//! Parses time from narrow character string and returns new DateTime object from it
	/*!
	    TODO Milliseconds support
	*/
	static DateTime fromString(const std::string& str, const std::string& fmt = std::string(IsoInputFormat));
	//! Parses time from wide character string and returns new DateTime object from it
	/*!
	    TODO Milliseconds support
	*/
	static DateTime fromWString(const std::wstring& str, const std::wstring& fmt = std::wstring(IsoInputWFormat));
private:
	enum Consts {
		FormatBufferSize = 4096
	};

	Date _date;
	Time _time;
};

} // namespace isl

#endif


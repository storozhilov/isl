#ifndef ISL__BASIC_DATE_TIME__HXX
#define ISL__BASIC_DATE_TIME__HXX

#include <time.h>
#include <sstream>
#include <iomanip>

namespace isl
{

//! Base class for all datetime staff
class BasicDateTime
{
public:
	//! Fundamental datetime constants
	enum Consts {
		SecondsPerDay = 86400,
		SecondsPerHour = 3600,
		SecondsPerMinute = 60,
		FormatBufferSize = 4096
	};
	//! Returns an empty UNIX breakdown time structure
	static struct tm emptyBdts();
	//! Resets UNIX breakdown time structure
	/*!
	  \param bdts UNIX breakdown time structure to reset
	*/
	static void resetBdts(struct tm& bdts);
	//! Returns an empty libc timespec structure
	static struct timespec emptyTimeSpec();
	//! Makes a libc timespec structure
	/*!
	  \param sec Seconds
	  \param nsec Nanoseconds
	  \return libc timespec structure
	*/
	static struct timespec makeTimeSpec(time_t sec, long int nsec);
	//! Returns current timestamp as libc timespec structure
	static struct timespec nowTimeSpec();
	//! Resets libc timespec structure
	/*!
	  \param ts libc timespec structure to reset
	*/
	static void resetTimeSpec(struct timespec& ts);
	//! Inspects for leap year
	/*!
	    \param year Year to inspect
	    \return TRUE if a leap year has been provided
	*/
	static bool isLeapYear(int year);
	//! Validates date
	/*!
	    \param year Year
	    \param month Month
	    \param day Day
	    \return TRUE if arguments replresent a valid date
	*/
	static bool isValidDate(int year, int month, int day);
	//! Validates time
	/*!
	    \param hour Hour
	    \param minute Minute
	    \param second Second
	    \param nanoSecond Nanosecond
	    \return TRUE if arguments replresent a valid time
	*/
	static bool isValidTime(int hour, int minute, int second, int nanoSecond = 0)
	{
		return (0 <= hour) && (hour <= 23) && (0 <= minute) && (minute <= 59) && (0 <= second) && (second <= 59) &&
			(0 <= nanoSecond) && (nanoSecond <= 999999999);
	}
protected:
	//! Converts string to UNIX breakdown time structure using the supplied format
	/*!
	  \param str String to parse
	  \param fmt Format (see <tt>man strptime</tt> plus '%f' field for nanosecond)
	  \param bdts UNIX breakdown time structure to put the result in
	  \param nanoSecond Nanoseconds to put the result in
	  \return TRUE if no parsing error occured
	*/
	static bool str2bdts(const std::string& str, const std::string& fmt, struct tm& bdts, int& nanoSecond);
	//! Converts UNIX breakdown time structure to string using the supplied format
	/*!
	  \param bdts UNIX breakdown time structure to convert
	  \param nanoSecond Nanoseconds to convert
	  \param fmt Format (see <tt>man strptime</tt> plus '%f' field for nanosecond)
	  \param str String to put the result in
	  \return TRUE if no conversion error occured
	*/
	static bool bdts2str(const struct tm& bdts, int nanoSecond, const std::string& fmt, std::string& str);
private:
	const static int _monthDays[];
};

//! POSIX.1b structure comparison operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return TRUE if comparison is truly
*/
bool operator==(const struct timespec& lhs, const struct timespec& rhs);
//! POSIX.1b structure comparison operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return TRUE if comparison is truly
*/
bool operator!=(const struct timespec& lhs, const struct timespec& rhs);
//! POSIX.1b structure comparison operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return TRUE if comparison is truly
*/
bool operator<(const struct timespec& lhs, const struct timespec& rhs);
//! POSIX.1b structure comparison operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return TRUE if comparison is truly
*/
bool operator<=(const struct timespec& lhs, const struct timespec& rhs);
//! POSIX.1b structure comparison operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return TRUE if comparison is truly
*/
bool operator>(const struct timespec& lhs, const struct timespec& rhs);
//! POSIX.1b structure comparison operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return TRUE if comparison is truly
*/
bool operator>=(const struct timespec& lhs, const struct timespec& rhs);
//! POSIX.1b structure arithmetic operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return Arithmetic operatoin result
*/
struct timespec operator+(const struct timespec& lhs, const struct timespec& rhs);
//! POSIX.1b structure arithmetic operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return Arithmetic operatoin result
*/
struct timespec& operator+=(struct timespec& lhs, const struct timespec& rhs);
//! POSIX.1b structure arithmetic operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return Arithmetic operatoin result
*/
struct timespec operator-(const struct timespec& lhs, const struct timespec& rhs);
//! POSIX.1b structure arithmetic operator
/*!
  \param lhs Left handed side argument
  \param rhs Right handed side argument
  \return Arithmetic operatoin result
*/
struct timespec& operator-=(struct timespec& lhs, const struct timespec& rhs);

//! POSIX.1b time structure comparison helper class
/*!
  Use it in <tt>std::map</tt> or <tt>std::multimap</tt> instantiation when
  <tt>struct timespec</tt> value is used as key.
*/
class TimeSpecComp
{
public:
	//! Comparison &quot;less than&quot; operator
	/*!
	  \param lhs Left handed side argument
	  \param rhs Right handed side argument
	  \return TRUE if lhs is less than rhs
	*/
	inline bool operator()(const struct timespec& lhs, const struct timespec& rhs)
	{
		return lhs < rhs;
	}
};

} // namespace isl

#endif

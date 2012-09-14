#ifndef ISL__TIME_ZONE__HXX
#define ISL__TIME_ZONE__HXX

#include <isl/BasicDateTime.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <errno.h>

namespace isl
{

//! Timezone class
class TimeZone : public BasicDateTime
{
public:
	//! Constructs default (GMT) timezone
	TimeZone() :
		_gmtOffset(0),
		_isDst(false)
	{}
	//! Constructs timezone
	/*!
	  \param gmtOffset GMT-offset
	  \param isDst DST flag
	*/
	TimeZone(long int gmtOffset, bool isDst = false) :
		_gmtOffset(normalizeGmtOffset(gmtOffset)),
		_isDst(isDst)
	{}
	//! Constructs timezone from the UNIX breakdown time structure
	/*!
	  \param bdts UNIX breakdown time structure
	*/
	TimeZone(const struct tm& bdts) :
		_gmtOffset(normalizeGmtOffset(bdts.tm_gmtoff)),
		_isDst(bdts.tm_isdst > 0)
	{}
	//! Returns GMT-offset
	inline long int gmtOffset() const
	{
		return _gmtOffset;
	}
	//! Returns DST flag
	inline bool isDst() const
	{
		return _isDst;
	}
	//! Applies timezone information to the UNIX breakdown time structure
	/*!
	  \param bdts UNIX breakdown time structure to apply to
	*/
	inline void apply(struct tm& bdts) const
	{
		bdts.tm_gmtoff = _gmtOffset;
		bdts.tm_isdst = _isDst ? 1 : 0;
		//bdts.tm_zone = 0;		// TODO ???
	}
	//! Returns GMT timezone
	static TimeZone gmt()
	{
		return TimeZone(0, false);
	}
	//! Returns current local timezone
	static TimeZone local()
	{
		static TimeZone cltz = currentLocalTimezone();
		return cltz;
	}
	//! Returns local timezone at the specified GMT-timestamp
	/*!
	  \param gmtSecond GMT-seconds from Epoch
	*/
	static TimeZone local(time_t gmtSecond)
	{
		struct tm bdts;
		if (!localtime_r(&gmtSecond, &bdts)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::LocalTimeR, errno, "Error converting time_t to local time"));
		}
		return TimeZone(bdts);
	}
	//! Comparison operator
	/*!
	  \param rhs Another timezone value to compare with
	*/
	bool operator==(const TimeZone& rhs) const
	{
		return _gmtOffset == rhs._gmtOffset && _isDst == rhs._isDst;
	}
private:
	static TimeZone currentLocalTimezone()
	{
		time_t now = time(0);
		struct tm bdts;
		if (!localtime_r(&now, &bdts)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::LocalTimeR, errno, "Error converting time_t to local time"));
		}
		return TimeZone(bdts);
	}
	static long int normalizeGmtOffset(long int gmtOffset)
	{
		gmtOffset %= SecondsPerDay;
		if (gmtOffset < SecondsPerDay / -2) {
			return SecondsPerDay + gmtOffset;
		} else if (SecondsPerDay / 2 < gmtOffset) {
			return gmtOffset - SecondsPerDay;
		} else {
			return gmtOffset;
		}
	}

	long int _gmtOffset;
	bool _isDst;
};

} // namespace isl

#endif

#include <isl/Timestamp.hxx>

namespace isl
{

Timestamp::Timestamp(time_t sec, long int nsec) :
	_ts(TimeSpec::makeTimestamp(sec, nsec))
{}

Timestamp::Timestamp(const struct timespec& ts) :
	_ts(TimeSpec::makeTimestamp(ts))
{}

Timeout Timestamp::leftTo() const
{
	timespec now = TimeSpec::now();
	if (now.tv_sec > _ts.tv_sec || (now.tv_sec == _ts.tv_sec && now.tv_nsec >= _ts.tv_nsec)) {
		// Time has been elapsed
		return Timeout();
	} else {
		return Timeout(
			_ts.tv_nsec >= now.tv_nsec ? _ts.tv_sec - now.tv_sec : _ts.tv_sec - now.tv_sec - 1,
			_ts.tv_nsec >= now.tv_nsec ? _ts.tv_nsec - now.tv_nsec : _ts.tv_nsec + 1000000000L - now.tv_nsec
		);
	}
}

Timestamp Timestamp::limit(const Timeout& timeout)
{
	return Timestamp::now() + timeout;
}

} // namespace isl

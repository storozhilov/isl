#include <isl/Ticker.hxx>
#include <isl/TimeSpec.hxx>

namespace isl
{

Ticker::Ticker(const Timeout& timeout, bool tickOnIdle) :
	_timeout(timeout),
	_tickOnIdle(tickOnIdle),
	_nextTickLimit()
{}

const Timestamp& Ticker::tick(size_t * ticksExpired)
{
	if (ticksExpired) {
		*ticksExpired = 0;
	}
	Timestamp now = Timestamp::now();
	if (_nextTickLimit.isZero()) {
		// First tick
		_nextTickLimit = now + _timeout;
		return _nextTickLimit;
	}
	if (now < _nextTickLimit) {
		// Idling - next tick limit have not been reached
		if (_tickOnIdle) {
			_nextTickLimit += _timeout;
		}
		return _nextTickLimit;
	}
	while (_nextTickLimit <= now) {
		_nextTickLimit += _timeout;
		if (ticksExpired) {
			++(*ticksExpired);
		}
	}
	return _nextTickLimit;
}

} // namespace isl

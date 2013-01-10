#include <isl/Ticker.hxx>
#include <isl/TimeSpec.hxx>

namespace isl
{

Ticker::Ticker(const Timeout& timeout) :
	_timeout(timeout),
	_tickStarted(TimeSpec::makeTimestamp()),
	_tickFinished(TimeSpec::makeTimestamp())
{}

Ticker::~Ticker()
{}

const Timestamp& Ticker::tick(size_t * ticksSkipped)
{
	if (ticksSkipped) {
		*ticksSkipped = 0;
	}
	Timestamp now = Timestamp::now();
	if (!_tickFinished.isZero() && ticksSkipped) {
		// Fetching skipped ticks amount
		Timestamp t = _tickFinished + _timeout;
		while (t <= now) {
			t += _timeout;
			++(*ticksSkipped);
		}
	}
	_tickStarted = now;
	_tickFinished = now + _timeout;
	return _tickFinished;
}

} // namespace isl

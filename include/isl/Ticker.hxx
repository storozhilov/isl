#ifndef ISL__CLOCK_TICKER
#define ISL__CLOCK_TICKER

#include <isl/Timestamp.hxx>
#include <isl/Timeout.hxx>

namespace isl
{

//! Clock ticker
class Ticker
{
public:
	//! Creates clock ticker
	/*!
	  \param ticker Clock ticker timeout
	  \param tickOnIdle To tick even if the next tick finalization timestamp have not been reached
	*/
	Ticker(const Timeout& timeout, bool tickOnIdle = false);
	//! Returnd clock ticker timeout
	inline const Timeout& timeout() const
	{
		return _timeout;
	}
	//! Returns 'tick on ilde' flag value
	inline bool tickOnIdle() const
	{
		return _tickOnIdle;
	}
	//! Returns next tick finalization timestamp or zero timestamp if the ticker have not been started
	inline const Timestamp& nextTickLimit() const
	{
		return _nextTickLimit;
	}
	//! Resets ticker
	inline void reset()
	{
		_nextTickLimit.reset();
	}
	//! Makes next tick
	/*!
	  \param ticksExpired Pointer where to place expired ticks amount or 0
	  \return Next tick finalization timestamp
	*/
	const Timestamp& tick(size_t * ticksExpired = 0);
private:
	Ticker();

	const Timeout _timeout;
	const bool _tickOnIdle;
	Timestamp _nextTickLimit;
};

} // namespace isl

#endif

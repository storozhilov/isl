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
	*/
	Ticker(const Timeout& timeout);
	virtual ~Ticker();
	//! Returnd clock ticker timeout
	inline const Timeout& timeout() const
	{
		return _timeout;
	}
	//! Returns tick starting timestamp
	inline const Timestamp& tickStarted() const
	{
		return _tickStarted;
	}
	//! Returns tick finalization timestamp
	inline const Timestamp& tickFinished() const
	{
		return _tickFinished;
	}
	inline void reset()
	{
		_tickStarted.reset();
		_tickFinished.reset();
	}
	//! Makes next tick
	/*!
	  \param ticksSkipped Pointer where to place skipped ticks amount or 0
	  \return Next tick finalization timestamp
	*/
	const Timestamp& tick(size_t * ticksSkipped);
private:
	Ticker();

	Timeout _timeout;
	Timestamp _tickStarted;
	Timestamp _tickFinished;
};

} // namespace isl

#endif

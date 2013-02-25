#ifndef ISL__STREAM_LOG_TARGET__HXX
#define ISL__STREAM_LOG_TARGET__HXX

#include <isl/AbstractLogTarget.hxx>
#include <ostream>

namespace isl
{

//! Stream log target
class StreamLogTarget : public AbstractLogTarget
{
public:
	//! Constructs a stream log target
	/*!
	  \param logger Regerence to logging engine
	  \param stream Regerence to stream to write log messages to
	*/
	StreamLogTarget(AbstractLogger& logger, std::ostream& stream);
	
	//! Return reference to stream
	inline std::ostream& stream() const
	{
		return _stream;
	}

	//! Writes a log message
	/*!
	  \param msg Log message to write
	  \param prefix Log message prefix
	  \note Thread-unsafe
	*/
	virtual void log(const AbstractLogMessage& msg, const std::string& prefix);
private:
	std::ostream& _stream;
};

} // namespace isl

#endif

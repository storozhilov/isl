#include <isl/IOError.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractIOError
------------------------------------------------------------------------------*/

AbstractIOError::AbstractIOError(const std::wstring& message, SOURCE_LOCATION_ARGS_DECLARATION) :
	AbstractError(message, SOURCE_LOCATION_ARGS_PASSTHRU)
{}

/*------------------------------------------------------------------------------
 * TimeoutExpiredIOError
------------------------------------------------------------------------------*/

TimeoutExpiredIOError::TimeoutExpiredIOError(SOURCE_LOCATION_ARGS_DECLARATION) :
	AbstractIOError(L"Timeout expired on I/O device", SOURCE_LOCATION_ARGS_PASSTHRU)
{}
	
AbstractError * TimeoutExpiredIOError::clone() const
{
	return new TimeoutExpiredIOError(*this);
}

/*------------------------------------------------------------------------------
 * ConnectionAbortedIOError
------------------------------------------------------------------------------*/

ConnectionAbortedIOError::ConnectionAbortedIOError(SOURCE_LOCATION_ARGS_DECLARATION) :
	AbstractIOError(L"Connection aborted on I/O device", SOURCE_LOCATION_ARGS_PASSTHRU)
{}
	
AbstractError * ConnectionAbortedIOError::clone() const
{
	return new ConnectionAbortedIOError(*this);
}

/*------------------------------------------------------------------------------
 * DeviceIsNotOpenIOError
------------------------------------------------------------------------------*/

DeviceIsNotOpenIOError::DeviceIsNotOpenIOError(SOURCE_LOCATION_ARGS_DECLARATION) :
	AbstractIOError(L"I/O device is not open", SOURCE_LOCATION_ARGS_PASSTHRU)
{}
	
AbstractError * DeviceIsNotOpenIOError::clone() const
{
	return new DeviceIsNotOpenIOError(*this);
}

} // namespace isl


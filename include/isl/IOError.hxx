#ifndef ISL__IO_ERROR__HXX
#define ISL__IO_ERROR__HXX

#include <isl/AbstractError.hxx>

namespace isl
{

class AbstractIOError : public AbstractError
{
public:
	AbstractIOError(const std::wstring& message, SOURCE_LOCATION_ARGS_DECLARATION);
private:
	AbstractIOError();
};

class TimeoutExpiredIOError : public AbstractIOError
{
public:
	TimeoutExpiredIOError(SOURCE_LOCATION_ARGS_DECLARATION);
	
	virtual AbstractError * clone() const;
private:
	TimeoutExpiredIOError();
};

class ConnectionAbortedIOError : public AbstractIOError
{
public:
	ConnectionAbortedIOError(SOURCE_LOCATION_ARGS_DECLARATION);
	
	virtual AbstractError * clone() const;
private:
	ConnectionAbortedIOError();
};

class DeviceIsNotOpenIOError : public AbstractIOError
{
public:
	DeviceIsNotOpenIOError(SOURCE_LOCATION_ARGS_DECLARATION);
	
	virtual AbstractError * clone() const;
private:
	DeviceIsNotOpenIOError();
};

} // namespace isl

#endif


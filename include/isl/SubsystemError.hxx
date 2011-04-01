#ifndef ISL__SUBSYSTEM_ERROR__HXX
#define ISL__SUBSYSTEM_ERROR__HXX

#include <isl/AbstractError.hxx>

namespace isl
{

class AbstractSubsystemError : public AbstractError
{
public:
	AbstractSubsystemError(const std::wstring& message, SOURCE_LOCATION_ARGS_DECLARATION) :
		AbstractError(message, SOURCE_LOCATION_ARGS_PASSTHRU)
	{}
};

class CanNotChangeStateSubsystemError : public AbstractSubsystemError
{
public:
	CanNotChangeStateSubsystemError(const std::wstring& message, SOURCE_LOCATION_ARGS_DECLARATION) :
		AbstractSubsystemError(message, SOURCE_LOCATION_ARGS_PASSTHRU)
	{}
	
	virtual AbstractError * clone() const
	{
		return new CanNotChangeStateSubsystemError(*this);
	}
};

} // namespace isl

#endif


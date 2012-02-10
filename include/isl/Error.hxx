#ifndef ISL__ERROR__HXX
#define ISL__ERROR__HXX

#include <isl/AbstractError.hxx>

namespace isl
{

//! Basic error class for simple error reporting
class Error : public AbstractError
{
public:
	Error(SOURCE_LOCATION_ARGS_DECLARATION, const std::wstring& msg) :
		AbstractError(SOURCE_LOCATION_ARGS_PASSTHRU),
		_msg(msg)
	{}

	virtual AbstractError * clone() const
	{
		return new Error(*this);
	}
protected:
	virtual std::wstring composeMessage() const
	{
		return _msg;
	}
private:
	Error();

	std::wstring _msg;
};

} // namespace isl

#endif

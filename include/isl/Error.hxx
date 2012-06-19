#ifndef ISL__ERROR__HXX
#define ISL__ERROR__HXX

#include <isl/AbstractError.hxx>

namespace isl
{

//! Basic error class for simple error reporting
class Error : public AbstractError
{
public:
	Error(SOURCE_LOCATION_ARGS_DECLARATION, const std::string& msg) :
		AbstractError(SOURCE_LOCATION_ARGS_PASSTHRU),
		_msg(msg)
	{}

	virtual AbstractError * clone() const
	{
		return new Error(*this);
	}
private:
	Error();

	virtual std::string composeMessage() const
	{
		return _msg;
	}

	std::string _msg;
};

} // namespace isl

#endif

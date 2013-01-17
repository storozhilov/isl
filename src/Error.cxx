#include <isl/Error.hxx>

namespace isl
{

Error::Error(SOURCE_LOCATION_ARGS_DECLARATION, const std::string& msg) :
	AbstractError(SOURCE_LOCATION_ARGS_PASSTHRU),
	_msg()
{
	_msg << msg;
}

Error::Error(const Error& rhs) :
	AbstractError(rhs.file(), rhs.line(), rhs.function()),
	_msg()
{
	_msg << rhs._msg.str();
}

Error& Error::operator=(const Error& rhs)
{
	if (&rhs != this) {
		_msg.str("");
		_msg.clear();
		_msg << rhs._msg.str();
	}
	return *this;
}

AbstractError * Error::clone() const
{
	return new Error(*this);
}

std::string Error::composeMessage() const
{
	return _msg.str();
}

} // namespace isl

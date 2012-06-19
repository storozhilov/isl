#include <isl/Exception.hxx>

namespace isl
{

Exception::Exception(const AbstractError& error) :
	_errorAutoPtr(error.clone())
{}

Exception::Exception(const Exception& rhs) :
	_errorAutoPtr(rhs._errorAutoPtr->clone())
{}

Exception& Exception::operator=(const Exception& rhs)
{
	if (&rhs != this) {
		_errorAutoPtr.reset(rhs._errorAutoPtr->clone());
	}
	return *this;
}

Exception::~Exception() throw ()
{}

const char * Exception::what() const throw()
{
	return _errorAutoPtr->message().c_str();
}

} // namespace isl

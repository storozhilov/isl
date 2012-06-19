#ifndef ISL__EXCEPTION__HXX
#define ISL__EXCEPTION__HXX

#include <isl/AbstractError.hxx>
#include <exception>
#include <list>

#include <memory>

namespace isl
{

//! ISL exception
class Exception : public std::exception
{
public:
	//! Constructor
	/*!
	  \param error Error class object
	*/
	Exception(const AbstractError& error);
	//! Copying constructor
	/*!
	  \param rhs Exception to copy from
	*/
	Exception(const Exception& rhs);

	//! Assignment operator
	/*!
	  \param rhs Object to copy from
	  \return Reference to this object
	*/
	Exception& operator=(const Exception& rhs);
	//! Destructor
	virtual ~Exception() throw ();
	//! Returns pointer to error
	inline const AbstractError& error() const
	{
		return *_errorAutoPtr.get();
	}
	//! Overriding std::exception's virtual method
	virtual const char * what() const throw();
private:
	Exception();

	void resetErrors();

	std::auto_ptr<AbstractError> _errorAutoPtr;
};

} // namespace isl

#endif

#ifndef ISL__EXCEPTION__HXX
#define ISL__EXCEPTION__HXX

#include <isl/AbstractError.hxx>
#include <exception>
#include <list>

namespace isl
{

//! Exception, which can hold multiple errors
class Exception : public std::exception
{
public:
	typedef std::list<AbstractError *> ErrorList;
	//! Constructor
	/*!
	  \param error Error class object
	*/
	Exception(const AbstractError& error);
	//! Copying constructor
	/*!
	  \param rhs Exception to copy from
	*/
	Exception(const Exception& exception);
	//! Destructor
	~Exception() throw();

	//! Assignment operator
	/*!
	  \param rhs Object to copy from
	  \return Reference to this object
	*/
	Exception& operator=(const Exception& rhs);
	//! Adds an error to the exception
	/*!
	  \param error Error to add
	*/
	void addError(const AbstractError& error);
	//! Inspects for exception contains an error of the particular type
	template <typename T> bool hasError() const
	{
		for (ErrorList::const_iterator i = _errors.begin(); i != _errors.end(); ++i) {
			if ((*i)->instanceOf<T>()) {
				return true;
			}
		}
		return false;
	}
	//! Inspects for exception contain only one error of the particular type
	template <typename T> bool hasOneError() const
	{
		return (_errors.size() == 1) && (*_errors.begin())->instanceOf<T>();
	}
	//! Returns messages of the exception's errors
	const wchar_t * message() const throw();
	//! Returns debug information of the exception's errors
	const wchar_t * debug() const throw();
	//! std::exception compatibility support method
	virtual const char * what() const throw();
private:
	Exception();

	void resetErrors();

	ErrorList _errors;
	std::wstring _message;
	std::wstring _debug;
	std::string _what;
};

} // namespace isl

#endif

#ifndef ISL__ABSTRACT_ERROR__HXX
#define ISL__ABSTRACT_ERROR__HXX

#include <isl/Debug.hxx>
#include <isl/String.hxx>
#include <string>
#include <sstream>

namespace isl
{

//! Base class for usage in ISL's exception class (isl::Exception)
class AbstractError
{
public:
	//! Constructs an object
	/*!
	  You should use SOURCE_LOCATION_ARGS macro as a value for the first parameter

	  \param info Optional info about an error
	*/
	AbstractError(SOURCE_LOCATION_ARGS_DECLARATION, const std::string& info = std::string());
	//! Destructor
	virtual ~AbstractError();

	//! Returns source-code filename where error has been constructed
	inline const char * file() const throw()
	{
		return _file.c_str();
	}
	//! Returns source-code file line where error has been constructed
	inline unsigned int line() const throw()
	{
		return _line;
	}
	//! Returns source-code function name where error has been constructed
	inline const char * function() const throw()
	{
		return _function.c_str();
	}
	//! Returns error info
	inline const std::string& info() const
	{
		return _info;
	}
	//! Returns true if an error is an instance of particular type templated method
	template <typename T> bool instanceOf() const
	{
		return (dynamic_cast<const T *>(this));
	}
	//! Composes and returns an error message
	const std::string& message() const;
	//! Composing string with source code filename, line number and function name helper method
	std::string sourceLocation() const;

	//! Cloning method for mandatory overriding in subclasses.
	virtual AbstractError * clone() const = 0;
protected:
	//! Actually composes and returns an error message.
	virtual std::string composeMessage() const = 0;
private:
	AbstractError();

	std::string _file;
	unsigned int _line;
	std::string _function;
	mutable bool _isComposed;
	mutable std::string _message;
	std::string _info;
};

} // namespace isl

#endif

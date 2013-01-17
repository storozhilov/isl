#ifndef ISL__ERROR__HXX
#define ISL__ERROR__HXX

#include <isl/AbstractError.hxx>
#include <sstream>

namespace isl
{

//! Basic error class for simple error reporting
class Error : public AbstractError
{
public:
	//! Constructs an error
	/*!
	  \param SOURCE_LOCATION_ARGS_DECLARATION Put SOURCE_LOCATION_ARGS macro here
	  \param msg Error message
	*/
	Error(SOURCE_LOCATION_ARGS_DECLARATION, const std::string& msg);
	//! Copying constructor
	Error(const Error& rhs);

	//! Assignment operator
	Error& operator=(const Error& rhs);

	virtual AbstractError * clone() const;
	//! Stream output operator templated redefinition
	/*!
	  \param val Value to append to message text internal string stream
	  \return Reference to log error object
	  \tparam T Value type
	*/
	template <typename T> Error& operator<<(const T& val)
	{
		_msg << val;
		return *this;
	}
private:
	Error();

	virtual std::string composeMessage() const;

	std::ostringstream _msg;
};

} // namespace isl

#endif

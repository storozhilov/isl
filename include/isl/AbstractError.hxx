#ifndef ISL__ABSTRACT_ERROR__HXX
#define ISL__ABSTRACT_ERROR__HXX

#include <isl/Debug.hxx>
#include <isl/String.hxx>
#include <string>
#include <sstream>

namespace isl
{

//! Base class for usage in multi-error exception (isl::Exception)
class AbstractError
{
public:
	//! Constructs an object
	/*!
	    \param SOURCE_LOCATION_ARGS_DECLARATION 'SOURCE_LOCATION_ARGS' macro should be placed here while constructor call
	    \param info Error information
	*/
	AbstractError(SOURCE_LOCATION_ARGS_DECLARATION, const std::wstring& info = std::wstring()) :
		_messageComposed(false),
		_message(),
		_debug(),
		_what(),
		_file(SOURCE_LOCATION_ARGS_FILE),
		_line(SOURCE_LOCATION_ARGS_LINE),
		_function(SOURCE_LOCATION_ARGS_FUNCTION),
		_info(info)
	{}
	virtual ~AbstractError()
	{}

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
	//! Returns error information
	inline const wchar_t * info() const throw()
	{
		return _info.c_str();
	}
	//! Returns true if an error is an instance of particular type templated method
	template <typename T> bool instanceOf()
	{
		return (dynamic_cast<T *>(this));
	}
	//! Returns an error message
	inline const wchar_t * message() const
	{
		composeMessagePrivate();
		return _message.c_str();
	}
	//! Returns an error message with debugging information
	inline const wchar_t * debug() const
	{
		composeMessagePrivate();
		return _debug.c_str();
	}
	//! Returns an error message (for compatibility with std::exception class)
	inline const char * what() const
	{
		composeMessagePrivate();
		return _what.c_str();
	}

	//! Clones an error object.
	/*!
	    You should implement this method in the following manner:
<pre>...
AbstractError * YourErrorClass::clone() const
{
	return new YourErrorClass(*this);
}
...</pre>
	*/
	virtual AbstractError * clone() const = 0;
protected:
	//! Composing string with source code filename, line number and function name help function
	std::wstring composeSourceLocation() const
	{
		std::wostringstream sstr;
		sstr << String::utf8Decode(_file) << L'(' << _line << L"), " << String::utf8Decode(_function) << L": ";
		return sstr.str();
	}
	//! Appending info() to the error message helper function
	inline void appendInfo(std::wstring& msg) const
	{
		if (!_info.empty()) {
			msg += L": ";
			msg += _info;
		}
	}

	//! Actually composes and returns an error message.
	virtual std::wstring composeMessage() const = 0;
private:
	AbstractError();

	void composeMessagePrivate() const
	{
		if (!_messageComposed) {
			_message = composeMessage();
			_debug = composeSourceLocation() + _message;
			_what = String::utf8Encode(_message);
			_messageComposed = true;
		}
	}

	mutable bool _messageComposed;
	mutable std::wstring _message;
	mutable std::wstring _debug;
	mutable std::string _what;
	std::string _file;
	unsigned int _line;
	std::string _function;
	std::wstring _info;
};

} // namespace isl

#endif

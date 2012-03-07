#ifndef ISL__LOG_MESSAGE__HXX
#define ISL__LOG_MESSAGE__HXX

#include <isl/Exception.hxx>
#include <string>
#include <sstream>

namespace isl
{

//! Log message abstract class
class AbstractLogMessage
{
public:
	AbstractLogMessage() :
		_message(),
		_messageComposed(false)
	{}
	virtual ~AbstractLogMessage()
	{}

	//! Returns log message
	inline const wchar_t * message() const
	{
		if (!_messageComposed) {
			_message = composeMessage();
			_messageComposed = true;
		}
		return _message.c_str();
	}

	//! Composes & returns log message
	virtual std::wstring composeMessage() const = 0;
private:
	mutable std::wstring _message;
	mutable bool _messageComposed;
};

//! Basic log message (just "for love of the game")
class LogMessage : public AbstractLogMessage
{
public:
	LogMessage(const std::string& info) :
		AbstractLogMessage(),
		_info(String::utf8Decode(info))
	{}
	LogMessage(const std::wstring& info) :
		AbstractLogMessage(),
		_info(info)
	{}
	
	//! Composes and returns notification log message
	virtual std::wstring composeMessage() const
	{
		return _info;
	}
private:
	LogMessage();

	const std::wstring _info;
};

//! Debugging log message abstract class
class AbstractDebugLogMessage : public AbstractLogMessage
{
public:
	AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_DECLARATION) :
		AbstractLogMessage(),
		_file(SOURCE_LOCATION_ARGS_FILE),
		_line(SOURCE_LOCATION_ARGS_LINE),
		_function(SOURCE_LOCATION_ARGS_FUNCTION)
	{}

	//! Returns source code filename where the message was created
	inline const char * file() const throw()
	{
		return _file.c_str();
	}
	//! Returns source code line where the message was created
	inline unsigned int line() const throw()
	{
		return _line;
	}
	//! Returns source code function where the message was created
	inline const char * function() const throw()
	{
		return _function.c_str();
	}
protected:
	//! Composing string with source code filename, line number and function name help function
	std::wstring composeSourceLocation() const
	{
		std::wostringstream sstr;
		sstr << String::utf8Decode(_file) << L'(' << _line << L"), " << String::utf8Decode(_function) << L": ";
		return sstr.str();
	}
private:
	AbstractDebugLogMessage();

	const std::string _file;
	const unsigned int _line;
	const std::string _function;
};

//! Debugging log message class
class DebugLogMessage : public AbstractDebugLogMessage
{
public:
	DebugLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::string& info) :
		AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
		_info(String::utf8Decode(info))
	{}
	DebugLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::wstring& info) :
		AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
		_info(info)
	{}

	//! Composes and returns debug log message
	virtual std::wstring composeMessage() const
	{
		std::wstring result(composeSourceLocation());
		result += _info;
		return result;
	}
private:
	DebugLogMessage();

	const std::wstring _info;
};

//! Exception log message class
class ExceptionLogMessage : public AbstractDebugLogMessage
{
public:
	ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt) :
		AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
		_expt(expt),
		_info()
	{}
	ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt, const std::string& info) :
		AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
		_expt(expt),
		_info(String::utf8Decode(info))
	{}
	ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt, const std::wstring& info) :
		AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
		_expt(expt),
		_info(info)
	{}

	//! Composes and returns exception log message (TODO More informative composition?)
	virtual std::wstring composeMessage() const
	{
		std::wstring result(composeSourceLocation());
		if (!_info.empty()) {
			result += _info;
		} else if (dynamic_cast<const Exception *>(&_expt)) {
			result += L"isl::Exception instance has been thrown with following errors";
		} else {
			result += L"std::exception instance has been thrown with following errors";
		}
		result += L":\n";
		result += String::utf8Decode(_expt.what());
		return result;
	}
private:
	ExceptionLogMessage();

	const std::exception& _expt;
	const std::wstring _info;
};

} // namespace isl

#endif

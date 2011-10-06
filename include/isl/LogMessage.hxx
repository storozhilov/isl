#ifndef ISL__LOG_MESSAGE__HXX
#define ISL__LOG_MESSAGE__HXX

#include <isl/AbstractError.hxx>
#include <isl/Utf8TextCodec.hxx>
#include <isl/Exception.hxx>
#include <string>
#include <sstream>

namespace isl
{

//! Log message abstract class
class AbstractLogMessage
{
public:
	AbstractLogMessage()
	{}
	virtual ~AbstractLogMessage()
	{}

	//! Composes & returns log message
	virtual std::wstring compose() const = 0;
};

//! Basic log message (just "for love of the game")
class LogMessage : public AbstractLogMessage
{
public:
	LogMessage(const std::string& msg) :
		AbstractLogMessage(),
		_msg(Utf8TextCodec().decode(msg))
	{}
	LogMessage(const std::wstring& msg) :
		AbstractLogMessage(),
		_msg(msg)
	{}
	
	//! Composes and returns notification log message
	virtual std::wstring compose() const
	{
		return _msg;
	}
private:
	LogMessage();

	const std::wstring _msg;
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
	inline std::string file() const
	{
		return _file;
	}
	//! Returns source code line where the message was created
	inline unsigned int line() const
	{
		return _line;
	}
	//! Returns source code function where the message was created
	inline std::string function() const
	{
		return _function;
	}
protected:
	//! Composing string with source code filename, line number and function name help function
	std::wstring composeSourceLocation() const
	{
		std::wostringstream sstr;
		sstr << Utf8TextCodec().decode(_file) << L'(' << _line << L"), " << Utf8TextCodec().decode(_function) << L": ";
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
	DebugLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::string& msg) :
		AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
		_msg(Utf8TextCodec().decode(msg))
	{}
	DebugLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::wstring& msg) :
		AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
		_msg(msg)
	{}

	//! Composes and returns debug log message
	virtual std::wstring compose() const
	{
		std::wstring result(composeSourceLocation());
		result += _msg;
		return result;
	}
private:
	DebugLogMessage();

	const std::wstring _msg;
};

//! Error log message class
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
		_info(Utf8TextCodec().decode(info))
	{}
	ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt, const std::wstring& info) :
		AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
		_expt(expt),
		_info(info)
	{}

	//! Composes and returns debug log message
	virtual std::wstring compose() const
	{
		std::wstring result(composeSourceLocation());
		if (!_info.empty()) {
			result += _info;
		} else if (dynamic_cast<const Exception *>(&_expt)) {
			result += L"isl::Exception instance has been thrown";
		} else {
			result += L"std::exception instance has been thrown";
		}
		result += L":\n";
		result += Utf8TextCodec().decode(_expt.what());
		return result;
	}
private:
	ExceptionLogMessage();

	const std::exception& _expt;
	const std::wstring _info;
};

} // namespace isl

#endif

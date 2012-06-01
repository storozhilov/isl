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
	AbstractLogMessage();
	virtual ~AbstractLogMessage();

	//! Returns log message
	const wchar_t * message() const;
protected:
	//! Composes & returns log message
	virtual std::wstring compose() const = 0;
private:
	mutable std::wstring _message;
	mutable bool _isComposed;
};

//! Basic log message
class LogMessage : public AbstractLogMessage
{
public:
	LogMessage(const std::string& info);
	LogMessage(const std::wstring& info);
private:
	LogMessage();
	
	//! Composes and returns notification log message
	virtual std::wstring compose() const;

	const std::wstring _info;
};

//! Debugging log message abstract class
class AbstractDebugLogMessage : public AbstractLogMessage
{
public:
	AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_DECLARATION);

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
	std::wstring composeSourceLocation() const;
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
	DebugLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::string& info);
	DebugLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::wstring& info);
private:
	DebugLogMessage();

	//! Composes and returns debug log message
	virtual std::wstring compose() const;

	const std::wstring _info;
};

//! Exception log message class
class ExceptionLogMessage : public AbstractDebugLogMessage
{
public:
	ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt);
	ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt, const std::string& info);
	ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt, const std::wstring& info);
private:
	ExceptionLogMessage();

	//! Composes and returns exception log message
	virtual std::wstring compose() const;

	const std::exception& _expt;
	const std::wstring _info;
};

} // namespace isl

#endif

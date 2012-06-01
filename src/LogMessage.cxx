#include <isl/LogMessage.hxx>

namespace isl
{

AbstractLogMessage::AbstractLogMessage() :
	_message(),
	_isComposed(false)
{}

AbstractLogMessage::~AbstractLogMessage()
{}

const wchar_t * AbstractLogMessage::message() const
{
	if (!_isComposed) {
		_message = compose();
		_isComposed = true;
	}
	return _message.c_str();
}

//------------------------------------------------------------------------------

LogMessage::LogMessage(const std::string& info) :
	AbstractLogMessage(),
	_info(String::utf8Decode(info))
{}

LogMessage::LogMessage(const std::wstring& info) :
	AbstractLogMessage(),
	_info(info)
{}
	
std::wstring LogMessage::compose() const
{
	return _info;
}

//------------------------------------------------------------------------------

AbstractDebugLogMessage::AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_DECLARATION) :
	AbstractLogMessage(),
	_file(SOURCE_LOCATION_ARGS_FILE),
	_line(SOURCE_LOCATION_ARGS_LINE),
	_function(SOURCE_LOCATION_ARGS_FUNCTION)
{}

std::wstring AbstractDebugLogMessage::composeSourceLocation() const
{
	std::wostringstream sstr;
	sstr << String::utf8Decode(_file) << L'(' << _line << L"), " << String::utf8Decode(_function) << L": ";
	return sstr.str();
}

//------------------------------------------------------------------------------

DebugLogMessage::DebugLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::string& info) :
	AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
	_info(String::utf8Decode(info))
{}

DebugLogMessage::DebugLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::wstring& info) :
	AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
	_info(info)
{}

std::wstring DebugLogMessage::compose() const
{
	std::wstring result(composeSourceLocation());
	result += _info;
	return result;
}

//------------------------------------------------------------------------------

ExceptionLogMessage::ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt) :
	AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
	_expt(expt),
	_info()
{}

ExceptionLogMessage::ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt, const std::string& info) :
	AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
	_expt(expt),
	_info(String::utf8Decode(info))
{}

ExceptionLogMessage::ExceptionLogMessage(SOURCE_LOCATION_ARGS_DECLARATION, const std::exception& expt, const std::wstring& info) :
	AbstractDebugLogMessage(SOURCE_LOCATION_ARGS_PASSTHRU),
	_expt(expt),
	_info(info)
{}

std::wstring ExceptionLogMessage::compose() const
{
	std::wstring result(composeSourceLocation());
	const Exception * e = dynamic_cast<const Exception *>(&_expt);
	if (!_info.empty()) {
		result += _info;
		result += L": ";
	} else if (e) {
		result += L"isl::Exception instance has been thrown with error";
		result += (e->errors().size() > 1) ? L"s:\n" : L": ";
	} else {
		result += L"std::exception instance has been thrown with error: ";
	}
	result += String::utf8Decode(_expt.what());
	return result;
}

} // namespace isl

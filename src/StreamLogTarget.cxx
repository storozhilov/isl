#include <isl/StreamLogTarget.hxx>
#include <isl/DateTime.hxx>

namespace isl
{

StreamLogTarget::StreamLogTarget(AbstractLogger& logger, std::ostream& stream) :
	AbstractLogTarget(logger),
	_stream(stream)
{}

void StreamLogTarget::log(const AbstractLogMessage& msg, const std::string& prefix)
{
	bool isFirstLine = true;
	size_t curPos = 0;
	do {
		size_t crlfPos = msg.message().find("\r\n", curPos);
		size_t crPos = msg.message().find("\n", curPos);
		size_t endlPos = (crlfPos < crPos) ? crlfPos : crPos;
		std::string currentLine = msg.message().substr(curPos, endlPos - curPos);
		curPos = (endlPos == std::string::npos) ? std::string::npos : ((crlfPos < crPos) ? endlPos + 2 : endlPos + 1);
		_stream << DateTime(msg.timestamp()).toString("%Y-%m-%d %H:%M:%S.%f") << (isFirstLine ? ": " : "> ");
		if (!prefix.empty()) {
			_stream << '[' << prefix << "] ";
		}
		_stream << composeSourceLocation(msg.file().c_str(), msg.line(), msg.function().c_str()) << ": ";
		_stream << currentLine << std::endl;
		isFirstLine = false;
	} while (curPos != std::string::npos);
}

} // namespace isl

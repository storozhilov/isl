#include <isl/AbstractQtScriptHTTPTask.hxx>

namespace isl
{

AbstractQtScriptHTTPTask::AbstractQtScriptHTTPTask(TcpSocket * socket) :
	AbstractHTTPTask(socket),
	_scriptEngine()
{}

} // namespace isl


#ifndef ISL__ABSTRACT_QT_SCRIPT_HTTP_TASK__HXX
#define ISL__ABSTRACT_QT_SCRIPT_HTTP_TASK__HXX

#include <isl/AbstractHTTPTask.hxx>
#include <QtScript/QScriptEngine>

namespace isl
{

class AbstractQtScriptHTTPTask : public AbstractHTTPTask
{
public:
	AbstractQtScriptHTTPTask(TcpSocket * socket);
private:
	AbstractQtScriptHTTPTask();
	AbstractQtScriptHTTPTask(const AbstractQtScriptHTTPTask&);				// No copy

	AbstractQtScriptHTTPTask& operator=(const AbstractQtScriptHTTPTask&);			// No copy

	QScriptEngine _scriptEngine;
};

} // namespace isl

#endif


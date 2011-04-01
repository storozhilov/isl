#ifndef ISL__TEST_QT__HXX
#define ISL__TEST_QT__HXX

#include <QtCore/QObject>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptable>
#include <QtScript/QScriptEngine>

namespace isl
{

class InvokableEcmaObject : public QObject, protected QScriptable
{
	Q_OBJECT
public:
	InvokableEcmaObject(QObject * parent = 0);

	Q_PROPERTY( QScriptValue value READ getValue )
private:
	QScriptValue getValue() const;
};

//------------------------------------------------------------------------------

class ConstructableEcmaObject : public QObject, protected QScriptable
{
	Q_OBJECT
public:
	ConstructableEcmaObject(QObject * parent = 0);

	static QScriptValue construct(QScriptContext * ctx, QScriptEngine * eng);

	Q_INVOKABLE QScriptValue getValue() const;
};

//------------------------------------------------------------------------------

class TestQt
{
public:
	TestQt();

	void execute();
private:
	QScriptEngine _eng;
	InvokableEcmaObject _invokable;
	ConstructableEcmaObject _constructable;
};

} // namespace isl

#endif

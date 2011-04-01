#include <iostream>

#include "TestQt.hxx"

namespace isl
{

InvokableEcmaObject::InvokableEcmaObject(QObject * parent) :
	QObject(parent),
	QScriptable()
{}

QScriptValue InvokableEcmaObject::getValue() const
{
	for (int i = 0; i < argumentCount(); ++i) {
		std::wcout << i << L"-th argument value is '" << argument(i).toString().toStdWString() << L"'" << std::endl;
	}
	return QScriptValue("Very important value from InvokableEcmaObject");
}

//------------------------------------------------------------------------------

ConstructableEcmaObject::ConstructableEcmaObject(QObject * parent) :
	QObject(parent),
	QScriptable()
{}

QScriptValue ConstructableEcmaObject::construct(QScriptContext * ctx, QScriptEngine * eng)
{
	if (ctx->isCalledAsConstructor()) {
		ctx->thisObject().setProperty("shit", QScriptValue(L"fuck"));
		return eng->undefinedValue();
	} else {
		QScriptValue object = eng->newObject();
		object.setPrototype(ctx->callee().property("prototype"));
		object.setProperty("shit", QScriptValue(L"fuck"));
		return object;
	}
}

QScriptValue ConstructableEcmaObject::getValue() const
{
	for (int i = 0; i < argumentCount(); ++i) {
		std::wcout << i << L"-th argument value is '" << argument(i).toString().toStdWString() << L"'" << std::endl;
	}
	return QScriptValue("Very important value from ConstructableEcmaObject");
}

//------------------------------------------------------------------------------

TestQt::TestQt() :
	_eng(),
	_invokable(),
	_constructable()
{
	QScriptValue invokableScriptValue = _eng.newQObject(&_invokable);
	_eng.globalObject().setProperty("Invokable", invokableScriptValue);
	QScriptValue constructablePrototype = _eng.newQObject(&_constructable);
	QScriptValue constructableFunction = _eng.newFunction(ConstructableEcmaObject::construct, constructablePrototype);
	_eng.globalObject().setProperty("Constructable", constructableFunction);
}

void TestQt::execute()
{
	//QScriptValue value = _eng.evaluate("Invokable.getValue(1, 'shit', new Object())");
	QScriptValue value = _eng.evaluate("Invokable.value");
	if (_eng.hasUncaughtException()) {
		std::wcerr << L"Evaluating caused an exceptioni: " << _eng.uncaughtException().toString().toStdWString() << std::endl;
	}
	std::wcout << L"value is '" << value.toString().toStdWString() << L"'" << std::endl;
	_eng.evaluate("Invokable.value = 'fuck'");
	if (_eng.hasUncaughtException()) {
		std::wcerr << L"Evaluating caused an exceptioni: " << _eng.uncaughtException().toString().toStdWString() << std::endl;
	}
	value = _eng.evaluate("Invokable.value");
	if (_eng.hasUncaughtException()) {
		std::wcerr << L"Evaluating caused an exceptioni: " << _eng.uncaughtException().toString().toStdWString() << std::endl;
	}
	std::wcout << L"value is '" << value.toString().toStdWString() << L"'" << std::endl;
	value = _eng.evaluate("Invokable.foo = 'bar'; Invokable.foo;");
	if (_eng.hasUncaughtException()) {
		std::wcerr << L"Evaluating caused an exceptioni: " << _eng.uncaughtException().toString().toStdWString() << std::endl;
	}
	std::wcout << L"Invokable.foo is '" << value.toString().toStdWString() << L"'" << std::endl;
	//QScriptValue obj = _eng.evaluate("new Invokable(1, 'shit', new Object())");
	//std::wcout << L"obj is '" << obj.toString().toStdWString() << L"'" << std::endl;
	//if (_eng.hasUncaughtException()) {
	//	std::wcerr << L"Evaluating caused an exception" << std::endl;
	//}
	value = _eng.evaluate("var obj = new Constructable(new Object(), 'fuck', 1); obj.getValue();");
	std::wcout << L"obj.getValue() is '" << value.toString().toStdWString() << L"'" << std::endl;
	if (_eng.hasUncaughtException()) {
		std::wcerr << L"Evaluating caused an exception" << std::endl;
	}
}

} // namespace isl

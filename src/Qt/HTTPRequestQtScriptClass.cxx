#include "HTTPRequestQtScriptClass.hxx"

namespace isl
{

/*------------------------------------------------------------------------------
  HTTPRequestQtScriptPrototype
------------------------------------------------------------------------------*/

//HTTPRequestQtScriptPrototype::HTTPRequestQtScriptPrototype(QObject * parent) :
//	QObject(parent),
//	QScriptable()
//{}
//
////QScriptValue HTTPRequestQtScriptPrototype::getValue() const
////{}

/*------------------------------------------------------------------------------
  HTTPRequestQtScriptClass
------------------------------------------------------------------------------*/

//HTTPRequestQtScriptClass::HTTPRequestQtScriptClass(QScriptEngine& engine, HTTPRequest& request) :
//	QObject(),
//	QScriptClass(&engine),
//	_request(request),
//	_prototype(),
//	_engine(engine),
//	_methodPropertyName(),
//	_uriPropertyName(),
//	_versionPropertyName()
//{
//	_prototype = engine.newQObject(new HTTPRequestQtScriptPrototype(this), QScriptEngine::ScriptOwnership,
//		QScriptEngine::SkipMethodsInEnumeration | QScriptEngine::ExcludeSuperClassMethods | QScriptEngine::ExcludeSuperClassProperties);
//	_prototype.setPrototype(engine.globalObject().property("Object").property("prototype"));
//	_methodPropertyName = engine.toStringHandle(QLatin1String("method"));
//	_uriPropertyName = engine.toStringHandle(QLatin1String("uri"));
//	_versionPropertyName = engine.toStringHandle(QLatin1String("version"));
//}
//
//QString HTTPRequestQtScriptClass::name() const
//{
//	return "Request";
//}
//
//QScriptValue HTTPRequestQtScriptClass::property(const QScriptValue& object, const QScriptString& name, uint id)
//{
//	if (name == _methodPropertyName) {
//		return _request.method().c_str();
//	}
//	if (name == _uriPropertyName) {
//		return _request.uri().c_str();
//	}
//	if (name == _versionPropertyName) {
//		return QScriptValue();
//	}
//	return QScriptValue();
//}
//
//QScriptValue::PropertyFlags HTTPRequestQtScriptClass::propertyFlags(const QScriptValue& object, const QScriptString& name, uint id)
//{
//}
//
//QScriptValue HTTPRequestQtScriptClass::prototype() const
//{
//	return _prototype;
//}
//
//QScriptClass::QueryFlags HTTPRequestQtScriptClass::queryProperty(const QScriptValue& object, const QScriptString& name,
//		QScriptClass::QueryFlags flags, uint * id)
//{
//}

HTTPRequestQtScriptClass::HTTPRequestQtScriptClass(HTTPRequest& request) :
	QObject(),
	QScriptable(),
	_request(request)
{}

QScriptValue HTTPRequestQtScriptClass::getMethod() const
{
	return QScriptValue(_request.method().value().asString().c_str());
}

QScriptValue HTTPRequestQtScriptClass::getUri() const
{
	return QScriptValue(_request.uri().c_str());
}

} // namespace isl


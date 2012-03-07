#ifndef ISL__HTTP_REQUEST_QT_SCRIPT_CLASS__HXX
#define ISL__HTTP_REQUEST_QT_SCRIPT_CLASS__HXX

#include <isl/HTTPRequest.hxx>
#include <QtCore/QObject>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptable>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptClass>

namespace isl
{

/*class HTTPRequestQtScriptPrototype : public QObject, protected QScriptable
{
	Q_OBJECT
public:
	HTTPRequestQtScriptPrototype(QObject * parent);

	//Q_INVOKABLE QScriptValue getValue() const;
private:
	//HTTPRequest& _request;
};

class HTTPRequestQtScriptClass : public QObject, public QScriptClass
{
public:
	HTTPRequestQtScriptClass(QScriptEngine& engine, HTTPRequest& request);

	virtual QString name() const;
	virtual QScriptValue property(const QScriptValue& object, const QScriptString& name, uint id);
	virtual QScriptValue::PropertyFlags propertyFlags(const QScriptValue& object, const QScriptString& name, uint id);
	virtual QScriptValue prototype() const;
	virtual QueryFlags queryProperty(const QScriptValue& object, const QScriptString& name, QueryFlags flags, uint * id);
private:
	HTTPRequest& _request;
	QScriptValue _prototype;
	QScriptEngine& _engine;
	QScriptString _methodPropertyName;
	QScriptString _uriPropertyName;
	QScriptString _versionPropertyName;
};*/

class HTTPRequestQtScriptClass : public QObject, protected QScriptable
{
	Q_OBJECT
public:
	HTTPRequestQtScriptClass(HTTPRequest& request);

	Q_PROPERTY( QScriptValue method READ getMethod )
	Q_PROPERTY( QScriptValue uri READ getUri )
private:
	QScriptValue getMethod() const;
	QScriptValue getUri() const;

	HTTPRequest& _request;
};

} // namespace isl

#endif


#ifndef ISL__VARIANT__HXX
#define ISL__VARIANT__HXX

#include <isl/DOMString.hxx>
#include <isl/DOMDocument.hxx>
#include <string>
#include <sstream>
#include <memory>

#include <stdexcept>								// TODO Remove it
#include <iostream>

namespace isl
{

/*------------------------------------------------------------------------------
 * VariantValue
 * ---------------------------------------------------------------------------*/

typedef std::wstring DOMDocumentTmp;

template <typename T> class VariantSerializer
{
public:
	static DOMDocumentTmp toXML(const T& value)
	{
		throw std::runtime_error("Serializing of this variant type is not implemented");
	}
	static T fromXML(DOMDocumentTmp xml)
	{
		throw std::runtime_error("Deserializing of this variant type is not implemented");
	}
};

template <> class VariantSerializer <int>
{
public:
	static DOMDocumentTmp toXML(const int& value)
	{
		/*DOMElement * rootElement = xml->getDocumentElement();
		DOMElement * typeElement = xml->createElement(DOMString("type").xmlString());
		rootElement->appendChild(typeElement);
		DOMText * typeText = xml->createTextNode(DOMString("numeric").xmlString());
		typeElement->appendChild(typeText);
		DOMElement * dataElement =  xml->createElement(DOMString("data").xmlString());
		rootElement->appendChild(dataElement);
		std::ostringstream sstr;
		sstr << value;
		DOMText * dataText = xml->createTextNode(DOMString(sstr.str().c_str()).xmlString());
		dataElement->appendChild(dataText);
		return;*/
		return DOMDocumentTmp();
	}
	static int fromXML(DOMDocumentTmp xml)
	{
		/*DOMNodeList * typeElements = xml->getElementsByTagName(DOMString("type").xmlString());
		if (typeElements->getLength() <= 0) {
			throw std::runtime_error("No <type> section in Variant's XML representation");
		}
		DOMNode * typeText = typeElements->item(0)->getFirstChild();
		if (!typeText) {
			throw std::runtime_error("No <type>[text]</type> section in Variant's XML representation");
		}
		if (!XMLString::equals(typeText->getNodeValue(), DOMString("numeric").xmlString())) {
			throw std::runtime_error("Converting from <type>numeric</type> type is supported only");
		}
		DOMNodeList * dataElements = xml->getElementsByTagName(DOMString("data").xmlString());
		if (dataElements->getLength() <= 0) {
			throw std::runtime_error("No <data> section in Variant's XML representation");
		}
		DOMNode * dataTextNode = dataElements->item(0)->getFirstChild();
		if (!dataTextNode) {
			throw std::runtime_error("No <data>[text]</data> section in Variant's XML representation");
		}
		std::wstring dataText = DOMString::stdWString(dataTextNode->getNodeValue());
		std::wstringstream sstr;
		sstr << dataText;
		int result;
		sstr >> result;
		return result;*/
		return 0;
	}
};

template <> class VariantSerializer <std::string>
{
public:
	static DOMDocumentTmp toXML(const std::string& value)
	{
		/*DOMElement * rootElement = xml->getDocumentElement();
		DOMElement * typeElement = xml->createElement(DOMString("type").xmlString());
		rootElement->appendChild(typeElement);
		DOMText * typeText = xml->createTextNode(DOMString("string").xmlString());
		typeElement->appendChild(typeText);
		DOMElement * dataElement =  xml->createElement(DOMString("data").xmlString());
		rootElement->appendChild(dataElement);
		//DOMText * dataTextNode = xml->createTextNode(DOMString(value));
		//dataElement->appendChild(dataTextNode);
		return;*/
		return DOMDocumentTmp();
	}
	static int fromXML(DOMDocumentTmp xml)
	{
		/*DOMNodeList * typeElements = xml->getElementsByTagName(DOMString("type").xmlString());
		if (typeElements->getLength() <= 0) {
			throw std::runtime_error("No <type> section in Variant's XML representation");
		}
		DOMNode * typeText = typeElements->item(0)->getFirstChild();
		if (!typeText) {
			throw std::runtime_error("No <type>[text]</type> section in Variant's XML representation");
		}
		if (!XMLString::equals(typeText->getNodeValue(), DOMString("numeric").xmlString())) {
			throw std::runtime_error("Converting from <type>numeric</type> type is supported only");
		}
		DOMNodeList * dataElements = xml->getElementsByTagName(DOMString("data").xmlString());
		if (dataElements->getLength() <= 0) {
			throw std::runtime_error("No <data> section in Variant's XML representation");
		}
		DOMNode * dataTextNode = dataElements->item(0)->getFirstChild();
		if (!dataTextNode) {
			throw std::runtime_error("No <data>[text]</data> section in Variant's XML representation");
		}
		std::wstring dataText = DOMString::stdWString(dataTextNode->getNodeValue());
		std::wstringstream sstr;
		sstr << dataText;
		int result;
		sstr >> result;
		return result;*/
		return 0;
	}
};

template <typename T> class VariantOperator
{
public:
	static std::wstring serialize(const T& value)
	{
		throw std::runtime_error("Serializing of this variant type is not implemented");
	}
	static T deserialize(const std::wstring serializedValue)
	{
		throw std::runtime_error("Deserializing of this variant type is not implemented");
	}
// TODO
//	static AbstractVariantFormatter * createFormatter()
//	{
//	}
};

template <> class VariantOperator <int>
{
public:
	static std::wstring serialize(const int& value)
	{
		std::wostringstream oss;
		oss << value;
		return oss.str();
	}
	static int deserialize(const std::wstring serializedValue)
	{
		int result;
		std::wistringstream iss(serializedValue);
		iss >> result;
		return result;
	}
};

template <> class VariantOperator <std::wstring>
{
public:
	static std::wstring serialize(const std::wstring& value)
	{
		return value;
	}
	static std::wstring deserialize(const std::wstring serializedValue)
	{
		return serializedValue;
	}
};

/*------------------------------------------------------------------------------
 * Variant
 * ---------------------------------------------------------------------------*/

/*class Variant
{
public:
	Variant() :
		_isNull(true),
		_xml()
	{}
	template <typename T> Variant(const T& value) :
		_isNull(true),
		_xml(VariantSerializer<T>::toXML(value))
	{}

	bool isNull() const
	{
		return _isNull;
	}
	template <typename T> T value() const
	{
		return VariantSerializer<T>::fromXML(_xml);
	}
private:
	Variant(const Variant&);

	Variant& operator=(const Variant&);
	
	bool _isNull;
	DOMDocumentTmp _xml;
};*/

class Variant
{
public:
	Variant() :
		_isNull(true),
		_serializedValue()
	{}
	template <typename T> Variant(const T& value) :
		_isNull(true),
		_serializedValue()
	{
		setValue(value);
	}

	bool isNull() const
	{
		return _isNull;
	}
	template <typename T> T value() const
	{
		return VariantOperator<T>::deserialize(_serializedValue);
	}
	template <typename T> void setValue(const T& newValue)
	{
		// TODO: Type Id, create formatter
		_serializedValue = VariantOperator<T>::serialize(newValue);
	}
private:
	Variant(const Variant&);

	Variant& operator=(const Variant&);
	
	bool _isNull;
	std::wstring _serializedValue;
};

} // namespace isl

#endif


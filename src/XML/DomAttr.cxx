#include <isl/DomAttr.hxx>
#include <isl/DomText.hxx>
#include <isl/DomError.hxx>
#include <isl/DomDocument.hxx>
#include <isl/Exception.hxx>

namespace isl
{

DomAttr::DomAttr(const DomString& name, bool specified, bool isId, DomDocument * ownerDocument, bool isReadOnly) :
	DomNode(DomNode::AttributeNode, ownerDocument, isReadOnly),
	_name(name),
	_namespaceURI(),
	_qualifiedName(),
	_prefix(),
	_localName(),
	_specified(specified),
	_isId(isId),
	_ownerElement(0)
{}

DomAttr::DomAttr(const DomString& namespaceURI, const DomString& qualifiedName, bool specified, bool isId,
			DomDocument * ownerDocument, bool isReadOnly) :
	DomNode(DomNode::AttributeNode, ownerDocument, isReadOnly),
	_name(),
	_namespaceURI(namespaceURI),
	_prefix(qualifiedName.parsePrefix()),
	_qualifiedName(qualifiedName),
	_localName(qualifiedName.parseLocalName()),
	_specified(specified),
	_isId(isId),
	_ownerElement(0)
{}

DomString DomAttr::name() const
{
	return (_localName.isNull()) ? _name : _qualifiedName;
}

bool DomAttr::specified() const
{
	return _specified;
}

DomString DomAttr::value() const
{
	DomString result;
	for (unsigned int i = 0; i < childNodes()->length(); ++i) {
		result += childNodes()->item(i)->nodeValue();
	}
	return result;
}

void DomAttr::setValue(const DomString& newValue)
{
	if (isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	bool textChildNodeFound = false;
	for (unsigned int i = 0; i < childNodes()->length(); ++i) {
		if (childNodes()->item(i)->nodeType() == TextNode) {
			if (textChildNodeFound) {
				dynamic_cast<DomText *>(childNodes()->item(i))->setData(DomString());
			} else {
				dynamic_cast<DomText *>(childNodes()->item(i))->setData(newValue);
				textChildNodeFound = true;
			}
		}
	}
	if (!textChildNodeFound) {
		DomText * newTextNode = ownerDocument()->createTextNode(newValue);
		appendChild(newTextNode);
	}
}

DomElement * DomAttr::ownerElement() const
{
	return _ownerElement;
}

//DomTypeInfo * DomAttr::schemaTypeInfo() const
//{}

bool DomAttr::isId() const
{
	return _isId;
}

DomString DomAttr::nodeName() const
{
	return name();
}

DomString DomAttr::nodeValue() const
{
	return value();
}

DomNamedNodeMap * DomAttr::attributes() const
{
	return 0;
}

DomString DomAttr::namespaceURI() const
{
	return _namespaceURI;
}

DomString DomAttr::prefix() const
{
	return _prefix;
}

DomString DomAttr::localName() const
{
	return _localName;
}

} // namespace isl


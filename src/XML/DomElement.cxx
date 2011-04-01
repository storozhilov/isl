#include <isl/DomElement.hxx>
#include <isl/DomAttr.hxx>
#include <isl/DomDocument.hxx>
#include <isl/DomError.hxx>
#include <isl/Exception.hxx>

namespace isl
{

DomElement::DomElement(const DomString& tagName, DomDocument * ownerDocument, bool isReadOnly) :
	DomNode(ElementNode, ownerDocument, isReadOnly),
	_tagName(tagName),
	_namespaceURI(),
	_qualifiedName(),
	_prefix(),
	_localName(),
	_attributes(this)
{}

DomElement::DomElement(const DomString& namespaceURI, const DomString& qualifiedName, DomDocument * ownerDocument, bool isReadOnly) :
	DomNode(ElementNode, ownerDocument, isReadOnly),
	_tagName(),
	_namespaceURI(namespaceURI),
	_qualifiedName(qualifiedName),
	_prefix(qualifiedName.parsePrefix()),
	_localName(qualifiedName.parseLocalName()),
	_attributes(this)
{}

DomString DomElement::tagName() const
{
	return _tagName;
}

DomString DomElement::getAttribute(const DomString& name) const
{
	DomNode * attr = _attributes.getNamedItem(name);
	return (attr) ? attr->nodeValue() : DomString();
}

DomString DomElement::getAttributeNS(const DomString& namespaceURI, const DomString& localName) const
{
	// TODO Throw following exceptions mentioned in http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core.html#ID-ElGetAttrNS:
	// * NOT_SUPPORTED_ERR
	DomNode * attr = _attributes.getNamedItemNS(namespaceURI, localName);
	return (attr) ? attr->nodeValue() : DomString();
}

DomAttr * DomElement::getAttributeNode(const DomString& name) const
{
	DomNode * attr = _attributes.getNamedItem(name);
	return (attr) ? dynamic_cast<DomAttr *>(attr) : 0;
}

DomAttr * DomElement::getAttributeNodeNS(const DomString& namespaceURI, const DomString& localName) const
{
	// TODO Throw following exceptions mentioned in http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core.html#ID-ElGetAtNodeNS
	// * NOT_SUPPORTED_ERR
	DomNode * attr = _attributes.getNamedItemNS(namespaceURI, localName);
	return (attr) ? dynamic_cast<DomAttr *>(attr) : 0;
}

DomNodeList DomElement::getElementsByTagName(const DomString& name) const
{
	DomNodeList elements;
	getElementsByTagNameRecursive(name, const_cast<DomElement * const>(this), elements);
	return elements;
}

DomNodeList DomElement::getElementsByTagNameNS(const DomString& namespaceURI, const DomString& localName) const
{
	// TODO Throw following exceptions mentioned in http://www.w3.org/TR/DOM-Level-3-Core/core.html#ID-A6C90942
	// * NOT_SUPPORTED_ERR
	DomNodeList elements;
	getElementsByTagNameNSRecursive(namespaceURI, localName, const_cast<DomElement * const>(this), elements);
	return elements;
}

bool DomElement::hasAttribute(const DomString& name) const
{
	return _attributes.getNamedItem(name);
}

bool DomElement::hasAttributeNS(const DomString& namespaceURI, const DomString& localName) const
{
	// TODO Throw following exceptions mentioned in http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core.html#ID-ElHasAttrNS:
	// * NOT_SUPPORTED_ERR
	return _attributes.getNamedItemNS(namespaceURI, localName);
}

void DomElement::removeAttribute(const DomString& name)
{
	// TODO Handle default attributes
	if (!_attributes.getNamedItem(name)) {
		return;
	}
	_attributes.removeNamedItem(name);
}

void DomElement::removeAttributeNS(const DomString& namespaceURI, const DomString& localName)
{
	// TODO Handle default attributes
	if (!_attributes.getNamedItemNS(namespaceURI, localName)) {
		return;
	}
	_attributes.removeNamedItemNS(namespaceURI, localName);
}

DomAttr * DomElement::removeAttributeNode(DomAttr * oldAttr)
{
	// TODO Handle default attributes
	_attributes.removeNode(oldAttr);						// NOTE! Exceptions are handled in this method
	return oldAttr;
}

void DomElement::setAttribute(const DomString& name, const DomString& value)
{
	// TODO Throw following exceptions mentioned in http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core.html#ID-F68F082:
	// * INVALID_CHARACTER_ERR
	if (isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	DomNode * attr = _attributes.getNamedItem(name);
	if (!attr) {
		attr = ownerDocument()->createAttribute(name);
		_attributes.setNamedItem(attr);
	}
	dynamic_cast<DomAttr *>(attr)->setValue(value);
}

void DomElement::setAttributeNS(const DomString& namespaceURI, const DomString& qualifiedName, const DomString& value)
{
	// TODO Throw following exceptions mentioned in http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core.html#ID-ElSetAttrNS:
	// * INVALID_CHARACTER_ERR
	// * NAMESPACE_ERR
	// * NOT_SUPPORTED_ERR
	DomNode * attr = _attributes.getNamedItemNS(namespaceURI, qualifiedName.parseLocalName());
	if (attr) {
		dynamic_cast<DomAttr *>(attr)->_prefix = qualifiedName.parsePrefix();
	} else {
		attr = ownerDocument()->createAttributeNS(namespaceURI, qualifiedName);
		_attributes.setNamedItemNS(attr);
	}
	dynamic_cast<DomAttr *>(attr)->setValue(value);
}

DomAttr * DomElement::setAttributeNode(DomAttr * newAttr)
{
	if (!newAttr) {
		throw Exception(DomError(DomError::InvalidAccessErr, SOURCE_LOCATION_ARGS));
	}
	DomNode * replacedAttr = _attributes.setNamedItem(newAttr);
	return (replacedAttr) ? dynamic_cast<DomAttr *>(replacedAttr) : 0;
}

DomAttr * DomElement::setAttributeNodeNS(DomAttr * newAttr)
{
	// TODO Throw following exceptions mentioned in http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core.html#ID-ElSetAtNodeNS
	// * NOT_SUPPORTED_ERR
	if (!newAttr) {
		throw Exception(DomError(DomError::InvalidAccessErr, SOURCE_LOCATION_ARGS));
	}
	DomNode * replacedAttr = _attributes.setNamedItemNS(newAttr);
	return (replacedAttr) ? dynamic_cast<DomAttr *>(replacedAttr) : 0;
}

void DomElement::setIdAttribute(const DomString& name, bool isId)
{
	if (isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	DomAttr * idAttr = getAttributeNode(name);
	if (!idAttr) {
		throw Exception(DomError(DomError::NotFoundErr, SOURCE_LOCATION_ARGS));
	}
	if (isId) {
		_attributes.resetIdAttributes();
	}
	idAttr->_isId = isId;
}

void DomElement::setIdAttributeNS(const DomString& namespaceURI, const DomString& localName, bool isId)
{
	if (isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	DomAttr * idAttr = getAttributeNodeNS(namespaceURI, localName);
	if (!idAttr) {
		throw Exception(DomError(DomError::NotFoundErr, SOURCE_LOCATION_ARGS));
	}
	if (isId) {
		_attributes.resetIdAttributes();
	}
	idAttr->_isId = isId;
}

void DomElement::setIdAttributeNode(DomAttr * idAttr, bool isId)
{
	if (isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	if (!_attributes.findNode(idAttr)) {
		throw Exception(DomError(DomError::NotFoundErr, SOURCE_LOCATION_ARGS));
	}
	if (isId) {
		_attributes.resetIdAttributes();
	}
	idAttr->_isId = isId;
}

DomString DomElement::nodeName() const
{
	return _tagName;
}

DomString DomElement::nodeValue() const
{
	return DomString();
}

DomNamedNodeMap * DomElement::attributes() const
{
	return const_cast<DomNamedNodeMap *>(&_attributes);
}

DomString DomElement::namespaceURI() const
{
	return _namespaceURI;
}

DomString DomElement::prefix() const
{
	return _prefix;
}

DomString DomElement::localName() const
{
	return _localName;
}

} // namespace isl


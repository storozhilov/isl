#include <isl/DomDocument.hxx>
#include <isl/DomImplementation.hxx>
#include <isl/DomNamedNodeMap.hxx>
#include <isl/DomElement.hxx>
#include <isl/DomText.hxx>
#include <isl/DomAttr.hxx>

namespace isl
{

DomDocument::DomDocument(const DomString& namespaceURI, const DomString& qualifiedName, DomImplementation * const implementation) :
	DomNode(DocumentNode, 0),
	_implementation(implementation),
	_namespaceURI(namespaceURI),
	_prefix(qualifiedName.parsePrefix()),
	_localName(qualifiedName.parseLocalName()),
	_nodes()
{}

DomDocument::~DomDocument()
{
	for (std::vector<DomNode *>::iterator i = _nodes.begin(); i != _nodes.end(); ++i) {
		delete (*i);
	}
}

DomImplementation * const DomDocument::implementation() const
{
	return _implementation;
}

DomElement * DomDocument::documentElement() const
{
	for (unsigned int i = 0; i < childNodes()->length(); ++i) {
		if (childNodes()->item(i)->nodeType() == DomNode::ElementNode) {
			return dynamic_cast<DomElement *>(childNodes()->item(i));
		}
	}
	return 0;
}

DomElement * DomDocument::createElement(const DomString& tagName)
{
	DomElement * newElementNode = new DomElement(tagName, this);
	_nodes.push_back(newElementNode);
	return newElementNode;
}

DomElement * DomDocument::createElementNS(const DomString& namespaceURI, const DomString& qualifiedName)
{
	DomElement * newElementNode = new DomElement(namespaceURI, qualifiedName, this);
	_nodes.push_back(newElementNode);
	return newElementNode;
}

DomText * DomDocument::createTextNode(const DomString& data)
{
	bool isElementContentWhitespace = true;					// TODO Use DomConfiguration "element-content-whitespace" parameter
	DomText * newTextNode = new DomText(data, isElementContentWhitespace, this);
	_nodes.push_back(newTextNode);
	return newTextNode;
}

DomAttr * DomDocument::createAttribute(const DomString& name)
{
	return createAttribute(name, true, false);
}

DomAttr * DomDocument::createAttributeNS(const DomString& namespaceURI, const DomString& qualifiedName)
{
	return createAttributeNS(namespaceURI, qualifiedName, true, false);
}

DomString DomDocument::nodeName() const
{
	return DomString(L"#document");
}

DomString DomDocument::nodeValue() const
{
	return DomString();
}

DomNamedNodeMap * DomDocument::attributes() const
{
	return 0;
}

DomString DomDocument::namespaceURI() const
{
	return _namespaceURI;
}

DomString DomDocument::prefix() const
{
	return _prefix;
}

DomString DomDocument::localName() const
{
	return _localName;
}

DomAttr * DomDocument::createAttribute(const DomString& name, bool specified, bool isId)
{
	DomAttr * newAttributeNode = new DomAttr(name, specified, isId, this);
	_nodes.push_back(newAttributeNode);
	return newAttributeNode;
}

DomAttr * DomDocument::createAttributeNS(const DomString& namespaceURI, const DomString& qualifiedName, bool specified, bool isId)
{
	DomAttr * newAttributeNode = new DomAttr(namespaceURI, qualifiedName, specified, isId, this);
	_nodes.push_back(newAttributeNode);
	return newAttributeNode;
}

} // namespace isl


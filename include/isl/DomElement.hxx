#ifndef ISL__DOM_ELEMENT__HXX
#define ISL__DOM_ELEMENT__HXX

#include <isl/DomNode.hxx>
#include <isl/DomNamedNodeMap.hxx>

namespace isl
{

class DomAttr;

class DomElement : public DomNode
{
public:
	DomString tagName() const;
	DomString getAttribute(const DomString& name) const;
	DomString getAttributeNS(const DomString& namespaceURI, const DomString& localName) const;
	DomAttr * getAttributeNode(const DomString& name) const;
	DomAttr * getAttributeNodeNS(const DomString& namespaceURI, const DomString& localName) const;
	DomNodeList getElementsByTagName(const DomString& name) const;						// TODO
	DomNodeList getElementsByTagNameNS(const DomString& namespaceURI, const DomString& localName) const;	// TODO
	bool hasAttribute(const DomString& name) const;
	bool hasAttributeNS(const DomString& namespaceURI, const DomString& localName) const;
	void removeAttribute(const DomString& name);
	void removeAttributeNS(const DomString& namespaceURI, const DomString& localName);
	DomAttr * removeAttributeNode(DomAttr * oldAttr);
	void setAttribute(const DomString& name, const DomString& value);
	void setAttributeNS(const DomString& namespaceURI, const DomString& qualifiedName, const DomString& value);
	DomAttr * setAttributeNode(DomAttr * newAttr);
	DomAttr * setAttributeNodeNS(DomAttr * newAttr);
	void setIdAttribute(const DomString& name, bool isId);
	void setIdAttributeNS(const DomString& namespaceURI, const DomString& localName, bool isId);
	void setIdAttributeNode(DomAttr * idAttr, bool isId);

	virtual DomString nodeName() const;
	virtual DomString nodeValue() const;
	virtual DomNamedNodeMap * attributes() const;
	virtual DomString namespaceURI() const;
	virtual DomString prefix() const;
	virtual DomString localName() const;
private:
	DomElement();
	DomElement(const DomElement&);
	DomElement(const DomString& tagName, DomDocument * ownerDocument, bool isReadOnly = false);
	DomElement(const DomString& namespaceURI, const DomString& qualifiedName, DomDocument * ownerDocument, bool isReadOnly = false);

	DomElement& operator=(const DomElement&);

	DomString _tagName;
	DomString _namespaceURI;
	DomString _qualifiedName;
	DomString _prefix;
	DomString _localName;
	DomNamedNodeMap _attributes;

	friend class DomDocument;
};

} // namespace isl

#endif


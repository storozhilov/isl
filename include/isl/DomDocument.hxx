#ifndef ISL__DOM_DOCUMENT__HXX
#define ISL__DOM_DOCUMENT__HXX

#include <isl/DomNode.hxx>

namespace isl
{

class DomImplementation;
class DomElement;
class DomText;
class DomAttr;

class DomDocument : public DomNode
{
public:
	virtual ~DomDocument();

	DomImplementation * const implementation() const;
	DomElement * documentElement() const;
	DomElement * createElement(const DomString& tagName);
	DomElement * createElementNS(const DomString& namespaceURI, const DomString& qualifiedName);
	DomText * createTextNode(const DomString& data);
	DomAttr * createAttribute(const DomString& name);
	DomAttr * createAttributeNS(const DomString& namespaceURI, const DomString& qualifiedName);

	virtual DomString nodeName() const;
	virtual DomString nodeValue() const;
	virtual DomNamedNodeMap * attributes() const;
	virtual DomString namespaceURI() const;
	virtual DomString prefix() const;
	virtual DomString localName() const;
private:
	DomDocument(const DomString& namespaceURI, const DomString& qualifiedName, DomImplementation * const implementation);
	DomDocument();
	DomDocument(const DomDocument&);

	DomDocument& operator=(const DomDocument&);

	DomAttr * createAttribute(const DomString& name, bool specified, bool isId);
	DomAttr * createAttributeNS(const DomString& namespaceURI, const DomString& qualifiedName, bool specified, bool isId);

	DomImplementation * const _implementation;
	DomString _namespaceURI;
	DomString _prefix;
	DomString _localName;
	std::vector<DomNode *> _nodes;

	friend class DomImplementation;
};

} // namespace isl

#endif


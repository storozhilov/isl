#ifndef ISL__DOM_ATTR__HXX
#define ISL__DOM_ATTR__HXX

#include <isl/DomNode.hxx>

namespace isl
{

class DomElement;

class DomAttr : public DomNode
{
public:
	DomString name() const;
	bool specified() const;
	DomString value() const;
	void setValue(const DomString& newValue);
	DomElement * ownerElement() const;
	//DomTypeInfo * schemaTypeInfo() const;
	bool isId() const;

	virtual DomString nodeName() const;
	virtual DomString nodeValue() const;
	virtual DomNamedNodeMap * attributes() const;
	virtual DomString namespaceURI() const;
	virtual DomString prefix() const;
	virtual DomString localName() const;
private:
	DomAttr();
	DomAttr(const DomAttr&);
	DomAttr(const DomString& name, bool specified, bool isId, DomDocument * ownerDocument, bool isReadOnly = false);
	DomAttr(const DomString& namespaceURI, const DomString& qualifiedName, bool specified, bool isId,
			DomDocument * ownerDocument, bool isReadOnly = false);

	DomAttr& operator=(const DomAttr&);

	DomString _name;
	DomString _namespaceURI;
	DomString _qualifiedName;
	DomString _prefix;
	DomString _localName;
	bool _specified;
	bool _isId;
	DomElement * _ownerElement;

	friend class DomDocument;
	friend class DomElement;
	friend class DomNamedNodeMap;
};

} // namespace isl

#endif


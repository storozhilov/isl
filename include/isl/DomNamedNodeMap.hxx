#ifndef ISL__DOM_NAMED_NODE_MAP__HXX
#define ISL__DOM_NAMED_NODE_MAP__HXX

#include <isl/DomString.hxx>
#include <map>

namespace isl
{

/*------------------------------------------------------------------------------
 * DomNamedNodeMap
 * ---------------------------------------------------------------------------*/

class DomNode;
class DomAttr;

class DomNamedNodeMap
{
public:
	DomNode * getNamedItem(const DomString& name) const;
	DomNode * getNamedItemNS(const DomString& namespaceURI, const DomString& localName) const;
	DomNode * setNamedItem(DomNode * node);
	DomNode * setNamedItemNS(DomNode * node);
	DomNode * removeNamedItem(const DomString& name);
	DomNode * removeNamedItemNS(const DomString& namespaceURI, const DomString& localName);
	DomNode * item(unsigned int index) const;
	unsigned int length() const;
private:
	DomNamedNodeMap(DomNode * ownerNode);
	DomNamedNodeMap();
	DomNamedNodeMap(const DomNamedNodeMap&);

	DomNamedNodeMap& operator=(const DomNamedNodeMap&);

	bool findNode(DomNode * node);
	void removeNode(DomNode * node);
	void handleSetItemExceptions(DomNode * node);
	void setOwner(DomNode * node);
	void resetOwner(DomNode * node);
	void resetIdAttributes();

	typedef DomString ItemKey;
	typedef std::pair<DomString, DomString> ItemKeyNS;
	typedef std::map<ItemKey, DomNode *> Items;
	typedef std::map<ItemKeyNS, DomNode *> ItemsNS;

	const DomNode * _ownerNode;
	Items _items;
	ItemsNS _itemsNS;

	friend class DomElement;
};

} // namespace isl

#endif


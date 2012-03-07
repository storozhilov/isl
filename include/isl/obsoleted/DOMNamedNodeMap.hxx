#ifndef ISL__DOM_NAMED_NODE_MAP__HXX
#define ISL__DOM_NAMED_NODE_MAP__HXX

#include <isl/DOMString.hxx>
#include <isl/DOMNode.hxx>
#include <isl/Reference.hxx>
#include <vector>

namespace isl
{

/*------------------------------------------------------------------------------
 * DOMNamedNodeMap
 * ---------------------------------------------------------------------------*/

class DOMNamedNodeMap
{
public:
	DOMNamedNodeMap();

	DOMNodeReference getNamedItem(const DOMString& name) const;
	DOMNodeReference getNamedItemNS(const DOMString& namespaceURI, const DOMString& localName) const;
	DOMNodeReference setNamedItem(const DOMNodeReference& node);
	DOMNodeReference setNamedItemNS(const DOMNodeReference& node);
	DOMNodeReference removeNamedItem(const DOMString& name);
	DOMNodeReference removeNamedItemNS(const DOMString& namespaceURI, const DOMString& localName);
	DOMNodeReference item(unsigned int index) const;
	unsigned int length() const;
private:
	typedef DOMNodeReference Item;
	typedef std::vector<Item> Items;

	Items _items;
};

typedef Reference<DOMNamedNodeMap> DOMNamedNodeMapReference;

} // namespace isl

#endif


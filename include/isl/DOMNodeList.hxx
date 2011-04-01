#ifndef ISL__DOM_NODE_LIST__HXX
#define ISL__DOM_NODE_LIST__HXX

#include <isl/Reference.hxx>
#include <isl/DOMNode.hxx>
#include <vector>

namespace isl
{

class DOMNodeList
{
public:
	DOMNodeList();

	DOMNodeReference item(unsigned int index) const;
	unsigned int length() const;
private:
	typedef DOMNodeReference Item;
	typedef std::vector<Item> Items;

	Items _items;
};

typedef Reference<DOMNodeList> DOMNodeListReference;

} // namespace isl

#endif


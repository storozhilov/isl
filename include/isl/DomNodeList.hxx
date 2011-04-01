#ifndef ISL__DOM_NODE_LIST__HXX
#define ISL__DOM_NODE_LIST__HXX

#include <vector>

namespace isl
{

class DomNode;

class DomNodeList
{
public:
	DomNodeList();

	DomNode * item(unsigned int index) const;
	unsigned int length() const;
private:
	typedef std::vector<DomNode *> Items;

	Items _items;

	friend class DomNode;
	friend class DomElement;
};

} // namespace isl

#endif


#ifndef ISL__DOM_NAME_LIST__HXX
#define ISL__DOM_NAME_LIST__HXX

#include <isl/Reference.hxx>
#include <isl/DOMString.hxx>
#include <vector>

namespace isl
{

class DOMNameList
{
public:
	DOMNameList();

	DOMString getName(unsigned int index) const;
	DOMString getNamespaceURI(unsigned int index) const;
	unsigned int length() const;
	bool contains(const DOMString& str) const;
	bool containsNS(const DOMString& namespaceURI, const DOMString& name) const;
private:
	typedef std::pair<DOMString, DOMString> Item;
	typedef std::vector<Item> Items;

	Items _items;
};

typedef Reference<DOMNameList> DOMNameListReference;

} // namespace isl

#endif


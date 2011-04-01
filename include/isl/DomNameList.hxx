#ifndef ISL__DOM_NAME_LIST__HXX
#define ISL__DOM_NAME_LIST__HXX

#include <isl/DomString.hxx>
#include <vector>

namespace isl
{

class DomNameList
{
public:
	DomNameList();

	DomString getName(unsigned int index) const;
	DomString getNamespaceURI(unsigned int index) const;
	unsigned int length() const;
	bool contains(const DomString& str) const;
	bool containsNS(const DomString& namespaceURI, const DomString& name) const;
private:
	typedef std::pair<DomString, DomString> Item;
	typedef std::vector<Item> Items;

	Items _items;
};

} // namespace isl

#endif


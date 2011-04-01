#ifndef ISL__DOM_STRING_LIST__HXX
#define ISL__DOM_STRING_LIST__HXX

#include <isl/Reference.hxx>
#include <isl/DOMString.hxx>
#include <vector>

namespace isl
{

class DOMStringList
{
public:
	DOMStringList();

	DOMString item(unsigned int index) const;
	unsigned int length() const;
	bool contains(const DOMString& str) const;
private:
	typedef std::vector<DOMString> Items;

	Items _items;
};

typedef Reference<DOMStringList> DOMStringListReference;

} // namespace isl

#endif


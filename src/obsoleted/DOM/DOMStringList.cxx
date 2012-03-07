#include <isl/DOMStringList.hxx>
#include <algorithm>

namespace isl
{

DOMStringList::DOMStringList() :
	_items()
{}

DOMString DOMStringList::item(unsigned int index) const
{
	if (index >= _items.size()) {
		return DOMString();
	} else {
		return _items[index];
	}
}

unsigned int DOMStringList::length() const
{
	return _items.size();
}

bool DOMStringList::contains(const DOMString& str) const
{
	return (std::find(_items.begin(), _items.end(), str) != _items.end());
}

} // namespace isl


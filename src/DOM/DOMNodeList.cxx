#include <isl/DOMNodeList.hxx>

namespace isl
{

DOMNodeList::DOMNodeList() :
	_items()
{}

DOMNodeReference DOMNodeList::item(unsigned int index) const
{
	if (index < _items.size()) {
		return _items[index];
	} else {
		return DOMNodeReference();
	}
}

unsigned int DOMNodeList::length() const
{
	return _items.size();
}

} // namespace isl


#include <isl/DomNodeList.hxx>
#include <isl/DomNode.hxx>

namespace isl
{

DomNodeList::DomNodeList() :
	_items()
{}

DomNode * DomNodeList::item(unsigned int index) const
{
	if (index < _items.size()) {
		return _items[index];
	} else {
		return 0;
	}
}

unsigned int DomNodeList::length() const
{
	return _items.size();
}

} // namespace isl


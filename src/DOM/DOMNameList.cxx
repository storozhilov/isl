#include <isl/DOMNameList.hxx>

namespace isl
{

DOMNameList::DOMNameList() :
	_items()
{}

DOMString DOMNameList::getName(unsigned int index) const
{
	if (index >= _items.size()) {
		return DOMString();
	} else {
		return _items[index].second;
	}
}

DOMString DOMNameList::getNamespaceURI(unsigned int index) const
{
	if (index >= _items.size()) {
		return DOMString();
	} else {
		return _items[index].first;
	}
}

unsigned int DOMNameList::length() const
{
	return _items.size();
}

bool DOMNameList::contains(const DOMString& str) const
{
	for (Items::const_iterator i = _items.begin(); i != _items.end(); ++i) {
		if ((*i).first.isNull() && (*i).second == str) {
			return true;
		}
	}
	return false;
}

bool DOMNameList::containsNS(const DOMString& namespaceURI, const DOMString& name) const
{
	for (Items::const_iterator i = _items.begin(); i != _items.end(); ++i) {
		if (((*i).first == namespaceURI) && ((*i).second == name)) {
			return true;
		}
	}
	return false;
}

} // namespace isl


#include <isl/DomNameList.hxx>

namespace isl
{

DomNameList::DomNameList() :
	_items()
{}

DomString DomNameList::getName(unsigned int index) const
{
	if (index >= _items.size()) {
		return DomString();
	} else {
		return _items[index].second;
	}
}

DomString DomNameList::getNamespaceURI(unsigned int index) const
{
	if (index >= _items.size()) {
		return DomString();
	} else {
		return _items[index].first;
	}
}

unsigned int DomNameList::length() const
{
	return _items.size();
}

bool DomNameList::contains(const DomString& str) const
{
	for (Items::const_iterator i = _items.begin(); i != _items.end(); ++i) {
		if ((*i).first.isNull() && (*i).second == str) {
			return true;
		}
	}
	return false;
}

bool DomNameList::containsNS(const DomString& namespaceURI, const DomString& name) const
{
	for (Items::const_iterator i = _items.begin(); i != _items.end(); ++i) {
		if (((*i).first == namespaceURI) && ((*i).second == name)) {
			return true;
		}
	}
	return false;
}

} // namespace isl


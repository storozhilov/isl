#include <isl/DOMNamedNodeMap.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * DOMNamedNodeMap
 * ---------------------------------------------------------------------------*/

DOMNamedNodeMap::DOMNamedNodeMap() :
	_items()
{}

DOMNodeReference DOMNamedNodeMap::getNamedItem(const DOMString& name) const
{
	if (name.isNull()) {
		return DOMNodeReference();
	}
	for (Items::const_iterator i = _items.begin(); i != _items.end(); ++i) {
		if ((*i)->nodeName() == name) {
			return (*i);
		}
	}
	return DOMNodeReference();
}

DOMNodeReference DOMNamedNodeMap::getNamedItemNS(const DOMString& namespaceURI, const DOMString& localName) const
{
	// TODO Throw following exceptions mentioned in http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core.html#ID-getNamedItemNS:
	// * NOT_SUPPORTED_ERR
	if (localName.isNull()) {
		return DOMNodeReference();
	}
	for (Items::const_iterator i = _items.begin(); i != _items.end(); ++i) {
		if (((*i)->localName() == localName) && ((*i)->namespaceURI() == namespaceURI)) {
			return (*i);
		}
	}
	return DOMNodeReference();
}


DOMNodeReference DOMNamedNodeMap::setNamedItem(const DOMNodeReference& node)
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

DOMNodeReference DOMNamedNodeMap::setNamedItemNS(const DOMNodeReference& node)
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

DOMNodeReference DOMNamedNodeMap::removeNamedItem(const DOMString& name)
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

DOMNodeReference DOMNamedNodeMap::removeNamedItemNS(const DOMString& namespaceURI, const DOMString& localName)
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

DOMNodeReference DOMNamedNodeMap::item(unsigned int index) const
{
	if (index < _items.size()) {
		return _items[index];
	} else {
		return DOMNodeReference();
	}
}

unsigned int DOMNamedNodeMap::length() const
{
	return _items.size();
}

} // namespace isl

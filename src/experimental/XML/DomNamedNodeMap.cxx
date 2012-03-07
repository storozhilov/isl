#include <isl/DomNamedNodeMap.hxx>
#include <isl/DomDocument.hxx>
#include <isl/DomNode.hxx>
#include <isl/DomElement.hxx>
#include <isl/DomAttr.hxx>
#include <isl/DomError.hxx>
#include <isl/Exception.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * DomNamedNodeMap
 * ---------------------------------------------------------------------------*/

DomNamedNodeMap::DomNamedNodeMap(DomNode * ownerNode) :
	_ownerNode(ownerNode),
	_items(),
	_itemsNS()
{
	if ((_ownerNode->nodeType() != DomNode::ElementNode) && (_ownerNode->nodeType() != DomNode::DocumentTypeNode)) {
		throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
	}
}

DomNode * DomNamedNodeMap::getNamedItem(const DomString& name) const
{
	if (name.isNull()) {
		return 0;
	}
	Items::const_iterator pos = _items.find(name);
	return (pos == _items.end()) ? 0 : (*pos).second;
}

DomNode * DomNamedNodeMap::getNamedItemNS(const DomString& namespaceURI, const DomString& localName) const
{
	// TODO Throw following exceptions mentioned in http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core.html#ID-getNamedItemNS:
	// * NOT_SUPPORTED_ERR
	if (localName.isNull()) {
		return 0;
	}
	ItemsNS::const_iterator pos = _itemsNS.find(ItemKeyNS(namespaceURI, localName));
	return (pos == _itemsNS.end()) ? 0 : (*pos).second;
}

DomNode * DomNamedNodeMap::setNamedItem(DomNode * node)
{
	handleSetItemExceptions(node);
	Items::iterator pos = _items.find(node->nodeName());
	if (pos == _items.end()) {
		setOwner(node);
		_items.insert(std::pair<ItemKey, DomNode *>(node->nodeName(), node));
		return 0;
	} else {
		if (node->isSameNode((*pos).second)) {
			return node;
		}
		DomNode * nodeToReplace = (*pos).second;
		setOwner(node);
		(*pos).second = node;
		resetOwner(nodeToReplace);
		return nodeToReplace;
	}
}

DomNode * DomNamedNodeMap::setNamedItemNS(DomNode * node)
{
	handleSetItemExceptions(node);
	ItemKeyNS mapKey(node->namespaceURI(), node->localName());
	ItemsNS::iterator pos = _itemsNS.find(mapKey);
	if (pos == _itemsNS.end()) {
		setOwner(node);
		_itemsNS.insert(std::pair<ItemKeyNS, DomNode *>(mapKey, node));
		return 0;
	} else {
		if (node->isSameNode((*pos).second)) {
			return node;
		}
		DomNode * nodeToReplace = (*pos).second;
		setOwner(node);
		(*pos).second = node;
		resetOwner(nodeToReplace);
		return nodeToReplace;
	}
}

DomNode * DomNamedNodeMap::removeNamedItem(const DomString& name)
{
	if (_ownerNode->isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	Items::iterator pos = _items.find(name);
	if (pos == _items.end()) {
		throw Exception(DomError(DomError::NotFoundErr, SOURCE_LOCATION_ARGS));
	}
	DomNode * nodeToRemove = (*pos).second;
	_items.erase(pos);
	resetOwner(nodeToRemove);
	return nodeToRemove;
}

DomNode * DomNamedNodeMap::removeNamedItemNS(const DomString& namespaceURI, const DomString& localName)
{
	// TODO Throw following exceptions mentioned in http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core.html#ID-removeNamedItemNS:
	// * NOT_SUPPORTED_ERR
	if (_ownerNode->isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	ItemKeyNS mapKey(namespaceURI, localName);
	ItemsNS::iterator pos = _itemsNS.find(mapKey);
	if (pos == _itemsNS.end()) {
		throw Exception(DomError(DomError::NotFoundErr, SOURCE_LOCATION_ARGS));
	}
	DomNode * nodeToRemove = (*pos).second;
	_itemsNS.erase(pos);
	resetOwner(nodeToRemove);
	return nodeToRemove;
}

DomNode * DomNamedNodeMap::item(unsigned int index) const
{
	if (index < _items.size()) {
		Items::const_iterator pos = _items.begin();
		for (; index > 0; --index) {
			++pos;
		}
		return (*pos).second;
	} else if (index < (_items.size() + _itemsNS.size())) {
		ItemsNS::const_iterator pos = _itemsNS.begin();
		index -= _items.size();
		for (; index > 0; --index) {
			++pos;
		}
		return (*pos).second;
	} else {
		return 0;
	}
}

unsigned int DomNamedNodeMap::length() const
{
	return (_items.size() + _itemsNS.size());
}

bool DomNamedNodeMap::findNode(DomNode * node)
{
	for (Items::iterator pos = _items.begin(); pos != _items.end(); ++pos) {
		if ((*pos).second->isSameNode(node)) {
			return true;
		}
	}
	for (ItemsNS::iterator pos = _itemsNS.begin(); pos != _itemsNS.end(); ++pos) {
		if ((*pos).second->isSameNode(node)) {
			return true;
		}
	}
	return false;
}

void DomNamedNodeMap::removeNode(DomNode * node)
{
	if (_ownerNode->isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	for (Items::iterator pos = _items.begin(); pos != _items.end(); ++pos) {
		if ((*pos).second->isSameNode(node)) {
			resetOwner(node);
			_items.erase(pos);
		}
	}
	for (ItemsNS::iterator pos = _itemsNS.begin(); pos != _itemsNS.end(); ++pos) {
		if ((*pos).second->isSameNode(node)) {
			resetOwner(node);
			_itemsNS.erase(pos);
		}
	}
	throw Exception(DomError(DomError::NotFoundErr, SOURCE_LOCATION_ARGS));
}

void DomNamedNodeMap::handleSetItemExceptions(DomNode * node)
{
	if (!node) {
		throw Exception(DomError(DomError::InvalidAccessErr, SOURCE_LOCATION_ARGS));
	}
	if (_ownerNode->isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	if (!_ownerNode->ownerDocument()->isSameNode(node->ownerDocument())) {
		throw Exception(DomError(DomError::WrongDocumentErr, SOURCE_LOCATION_ARGS));
	}
	if ((node->nodeType() == DomNode::AttributeNode) && dynamic_cast<DomAttr *>(node)->_ownerElement) {
		throw Exception(DomError(DomError::InUseAttributeErr, SOURCE_LOCATION_ARGS));
	}
	if (((_ownerNode->nodeType() == DomNode::ElementNode) && (node->nodeType() != DomNode::AttributeNode)) ||
	    ((_ownerNode->nodeType() == DomNode::DocumentTypeNode) && (node->nodeType() != DomNode::EntityNode))) {
		throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
	}
}

void DomNamedNodeMap::setOwner(DomNode * node)
{
	if (node->nodeType() == DomNode::AttributeNode) {
		dynamic_cast<DomAttr *>(node)->_ownerElement = dynamic_cast<DomElement *>(const_cast<DomNode *>(_ownerNode));
	//} else if (node->nodeType() == DomNode::EntityNode) {					// TODO
	//	dynamic_cast<DomEntity *>(node)->_ownerDocumentType = dynamic_cast<DomDocumentType *>(const_cast<DomNode *>(_ownerNode));;
	} else {
		throw Exception(DomError(DomError::InvalidNodeType, SOURCE_LOCATION_ARGS));
	}
}

void DomNamedNodeMap::resetOwner(DomNode * node)
{
	if (node->nodeType() == DomNode::AttributeNode) {
		dynamic_cast<DomAttr *>(node)->_ownerElement = 0;
	//} else if (node->nodeType() == DomNode::EntityNode) {					// TODO
	//	dynamic_cast<DomEntity *>(node)->_ownerDocumentType = 0;
	} else {
		throw Exception(DomError(DomError::InvalidNodeType, SOURCE_LOCATION_ARGS));
	}
}

void DomNamedNodeMap::resetIdAttributes()
{
	for (Items::iterator pos = _items.begin(); pos != _items.end(); ++pos) {
		DomAttr * attr = dynamic_cast<DomAttr *>((*pos).second);
		if (attr) {
			attr->_isId = false;
		}
	}
	for (ItemsNS::iterator pos = _itemsNS.begin(); pos != _itemsNS.end(); ++pos) {
		DomAttr * attr = dynamic_cast<DomAttr *>((*pos).second);
		if (attr) {
			attr->_isId = false;
		}
	}
}

} // namespace isl

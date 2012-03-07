#include <isl/DomNode.hxx>
#include <isl/DomDocument.hxx>
#include <isl/Exception.hxx>
#include <isl/DomError.hxx>
#include <algorithm>

namespace isl
{

/*------------------------------------------------------------------------------
 * DomNode
 * ---------------------------------------------------------------------------*/

DomNode::DomNode(NodeType nodeType, DomDocument * ownerDocument, bool isReadOnly) :
	_nodeType(nodeType),
	_ownerDocument(ownerDocument),
	_parentNode(0),
	_childNodes(),
	_isReadOnly(isReadOnly)
{}

DomNode::~DomNode()
{}

DomNode * DomNode::parentNode() const
{
	return _parentNode;
}

DomNode::NodeType DomNode::nodeType() const
{
	return _nodeType;
}

DomNodeList * DomNode::childNodes() const
{
	return const_cast<DomNodeList *>(&_childNodes);
}

DomNode * DomNode::firstChild() const
{
	if (_childNodes.length() == 0) {
		return 0;
	}
	return _childNodes.item(0);
}

DomNode * DomNode::lastChild() const
{
	if (_childNodes.length() == 0) {
		return 0;
	}
	return _childNodes.item(_childNodes.length() - 1);
}

DomNode * DomNode::previousSibling() const
{
	if (!_parentNode) {
		return 0;
	}
	DomNode * result(0);
	for (DomNodeList::Items::const_iterator i = _parentNode->_childNodes._items.begin(); i != _parentNode->_childNodes._items.end(); ++i) {
		if ((*i) == this) {
			break;
		} else {
			result = (*i);
		}
	}
	return result;
}

DomNode * DomNode::nextSibling() const
{
	if (!_parentNode) {
		return 0;
	}
	DomNode * result(0);
	for (DomNodeList::Items::const_reverse_iterator i = _parentNode->_childNodes._items.rbegin(); i != _parentNode->_childNodes._items.rend(); ++i) {
		if ((*i) == this) {
			break;
		} else {
			result = (*i);
		}
	}
	return result;
}

DomDocument * DomNode::ownerDocument() const
{
	return _ownerDocument;
}

DomNode * DomNode::insertBefore(DomNode * newChild, DomNode * refChild)
{
	// TODO Handle and throw following exceptions mentioned in http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core.html#ID-952280727
	// * NOT_SUPPORTED_ERR
	// Handling exceptions:
	if (isReadOnly() || ((newChild->_nodeType == DocumentFragmentNode) && newChild->_isReadOnly)) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	if (!newChild) {
		throw Exception(DomError(DomError::InvalidAccessErr, SOURCE_LOCATION_ARGS));
	}
	if (refChild && (refChild->_parentNode != this)) {
		throw Exception(DomError(DomError::NotFoundErr, SOURCE_LOCATION_ARGS));
	}
	if (((_nodeType == DocumentNode) && (newChild->_ownerDocument != this)) ||
	    ((_nodeType != DocumentNode) && (!_ownerDocument || (_ownerDocument != newChild->_ownerDocument)))) {
		throw Exception(DomError(DomError::WrongDocumentErr, SOURCE_LOCATION_ARGS));
	}
	switch (_nodeType) {
		case DocumentNode:
			if (newChild->_nodeType == ElementNode) {
				for (DomNodeList::Items::const_iterator i = _childNodes._items.begin(); i != _childNodes._items.end(); ++i) {
					if ((*i)->_nodeType == ElementNode) {
						throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
					}
				}
			} else if (newChild->_nodeType == DocumentTypeNode) {
				for (DomNodeList::Items::const_iterator i = _childNodes._items.begin(); i != _childNodes._items.end(); ++i) {
					if ((*i)->_nodeType == DocumentTypeNode) {
						throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
					}
				}
			} else if ((newChild->_nodeType != ProcessingInstructionNode) && (newChild->_nodeType != CommentNode)) {
				throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
			}
			break;
		case DocumentFragmentNode:
			if ((newChild->_nodeType != ElementNode) && (newChild->_nodeType != ProcessingInstructionNode) &&
			    (newChild->_nodeType != CommentNode) && (newChild->_nodeType != TextNode) &&
			    (newChild->_nodeType != CDataSectionNode) && (newChild->_nodeType != EntityReferenceNode)) {
				throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
			}
			break;
		case DocumentTypeNode:
			throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
			break;
		case EntityReferenceNode:
			if ((newChild->_nodeType != ElementNode) && (newChild->_nodeType != ProcessingInstructionNode) &&
			    (newChild->_nodeType != CommentNode) && (newChild->_nodeType != TextNode) &&
			    (newChild->_nodeType != CDataSectionNode) && (newChild->_nodeType != EntityReferenceNode)) {
				throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
			}
			break;
		case ElementNode:
			if ((newChild->_nodeType != ElementNode) && (newChild->_nodeType != TextNode) &&
			    (newChild->_nodeType != CommentNode) && (newChild->_nodeType != ProcessingInstructionNode) &&
			    (newChild->_nodeType != CDataSectionNode) && (newChild->_nodeType != EntityReferenceNode)) {
				throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
			}
			break;
		case AttributeNode:
			if ((newChild->_nodeType != TextNode) && (newChild->_nodeType != EntityReferenceNode)) {
				throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
			}
			break;
		case ProcessingInstructionNode:
			throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
			break;
		case CommentNode:
			throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
			break;
		case TextNode:
			throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
			break;
		case CDataSectionNode:
			throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
			break;
		case EntityNode:
			if ((newChild->_nodeType != ElementNode) && (newChild->_nodeType != ProcessingInstructionNode) &&
			    (newChild->_nodeType != CommentNode) && (newChild->_nodeType != TextNode) &&
			    (newChild->_nodeType != CDataSectionNode) && (newChild->_nodeType != EntityReferenceNode)) {
				throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
			}
			break;
		case NotationNode:
			throw Exception(DomError(DomError::HierarchyRequestErr, SOURCE_LOCATION_ARGS));
			break;
		default:
			throw Exception(DomError(DomError::InvalidNodeType, SOURCE_LOCATION_ARGS));
	}
	// Removing newChild from it's parent if needed:
	//if ((newChild->_nodeType != DocumentFragmentNode) && newChild->_parentNode) {		// ???
	if (newChild->_parentNode) {
		newChild->_parentNode->removeChild(newChild);
	}
	// Identifying the position in child nodes list to insert the newChild:
	DomNodeList::Items::iterator insertPosition = (refChild) ?
		std::find(_childNodes._items.begin(), _childNodes._items.end(), refChild) :
		_childNodes._items.end();
	// Extracting nodes to insert:
	std::vector<DomNode *> nodesToInsert;
	if (newChild->_nodeType == DocumentFragmentNode) {
		for (DomNodeList::Items::iterator i = newChild->_childNodes._items.begin(); i != newChild->_childNodes._items.end(); ++i) {
			(*i)->_parentNode = this;
			nodesToInsert.push_back(*i);
		}
		newChild->_childNodes._items.clear();
	} else {
		newChild->_parentNode = this;
		nodesToInsert.push_back(newChild);
	}
	// Inserting nodes and returning the result:
	_childNodes._items.insert(insertPosition, nodesToInsert.begin(), nodesToInsert.end());
	return newChild;
}

DomNode * DomNode::replaceChild(DomNode * newChild, DomNode * oldChild)
{
	// TODO Handle and throw following exceptions mentioned in http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core.html#ID-785887307
	// * NOT_SUPPORTED_ERR
	DomNode * oldChildNextSubling = (oldChild) ? oldChild->nextSibling() : 0;
	removeChild(oldChild);
	insertBefore(newChild, oldChildNextSubling);
}

DomNode * DomNode::removeChild(DomNode * oldChild)
{
	// TODO Handle and throw following exceptions mentioned in http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core.html#ID-1734834066
	// * NOT_SUPPORTED_ERR
	if (isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	if (!oldChild) {
		throw Exception(DomError(DomError::InvalidAccessErr, SOURCE_LOCATION_ARGS));
	}
	if (oldChild->_parentNode != this) {
		throw Exception(DomError(DomError::NotFoundErr, SOURCE_LOCATION_ARGS));
	}
	std::remove(_childNodes._items.begin(), _childNodes._items.end(), oldChild);
	oldChild->_parentNode = 0;
	return oldChild;
}

DomNode * DomNode::appendChild(DomNode * newChild)
{
	return insertBefore(newChild, 0);
}

bool DomNode::hasChildNodes() const
{
	return (_childNodes.length() > 0);
}

DomNode * DomNode::cloneNode(bool deep) const
{
	// TODO
	throw Exception(DomError(DomError::MethodNotImplemented, SOURCE_LOCATION_ARGS));
}

void DomNode::normalize()
{
	// TODO
	throw Exception(DomError(DomError::MethodNotImplemented, SOURCE_LOCATION_ARGS));
}

bool DomNode::isSupported(const DomString& feature, const DomString& version) const
{
	// TODO
	throw Exception(DomError(DomError::MethodNotImplemented, SOURCE_LOCATION_ARGS));
}

unsigned int DomNode::compareDocumentPosition(DomNode * other) const
{
	// TODO
	throw Exception(DomError(DomError::MethodNotImplemented, SOURCE_LOCATION_ARGS));
}

DomString DomNode::textContent() const
{
	// TODO
	throw Exception(DomError(DomError::MethodNotImplemented, SOURCE_LOCATION_ARGS));
}

void DomNode::setTextContent(const DomString& newContent)
{
	// TODO
	throw Exception(DomError(DomError::MethodNotImplemented, SOURCE_LOCATION_ARGS));
}

bool DomNode::isSameNode(DomNode * other) const
{
	return (this == other);
}

DomString DomNode::lookupPrefix() const
{
	// TODO
	throw Exception(DomError(DomError::MethodNotImplemented, SOURCE_LOCATION_ARGS));
}

bool DomNode::isDefaultNamespace(const DomString& namespaceURI) const
{
	// TODO
	throw Exception(DomError(DomError::MethodNotImplemented, SOURCE_LOCATION_ARGS));
}

DomString DomNode::lookupNamespaceURI(const DomString& prefix) const
{
	// TODO
	throw Exception(DomError(DomError::MethodNotImplemented, SOURCE_LOCATION_ARGS));
}

bool DomNode::isEqualNode(DomNode * other) const
{
	// TODO
	throw Exception(DomError(DomError::MethodNotImplemented, SOURCE_LOCATION_ARGS));
}

bool DomNode::isReadOnly() const
{
	return _isReadOnly;
}

void DomNode::getElementsByTagNameRecursive(const DomString& name, DomNode * const parent, DomNodeList& elements)
{
	for (unsigned int i = 0; i < parent->childNodes()->length(); ++i) {
		DomNode * child = parent->childNodes()->item(i);
		if ((child->nodeType() == ElementNode) && ((name == L"*") || (name == child->nodeName()))) {
			elements._items.push_back(child);
		}
		if ((child->nodeType() == DocumentNode) || (child->nodeType() == DocumentFragmentNode) ||
		    (child->nodeType() == EntityReferenceNode) || (child->nodeType() == ElementNode) ||
		    (child->nodeType() == EntityNode)) {
			getElementsByTagNameRecursive(name, child, elements);
		}
	}
}

void DomNode::getElementsByTagNameNSRecursive(const DomString& namespaceURI, const DomString& localName, DomNode * const parent,
		DomNodeList& elements)
{
	for (unsigned int i = 0; i < parent->childNodes()->length(); ++i) {
		DomNode * child = parent->childNodes()->item(i);
		if ((child->nodeType() == ElementNode) && ((namespaceURI == L"*") || (namespaceURI == child->namespaceURI())) &&
		    ((localName == L"*") || (localName == child->localName()))) {
			elements._items.push_back(child);
		}
		// DocumentNode and DocumentFragmentNode could not be the child, so they are not mentioned below
		if ((child->nodeType() == EntityReferenceNode) || (child->nodeType() == ElementNode) || (child->nodeType() == EntityNode)) {
			getElementsByTagNameNSRecursive(namespaceURI, localName, child, elements);
		}
	}
}

} // namespace isl


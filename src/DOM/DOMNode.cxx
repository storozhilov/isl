#include <isl/DOMNode.hxx>
#include <isl/DOMNodeList.hxx>
#include <isl/DOMNamedNodeMap.hxx>
#include <isl/DOMDocument.hxx>
#include <isl/Exception.hxx>
#include <isl/DOMError.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * DOMNode
 * ---------------------------------------------------------------------------*/

DOMNode::DOMNode(NodeType nodeType, const DOMDocumentReference& ownerDocument) :
	_nodeType(nodeType),
	_ownerDocument(ownerDocument),
	_parentNode(),
	_childNodes(new DOMNodeList())
{}

DOMNode::~DOMNode()
{}

DOMNode::NodeType DOMNode::nodeType() const
{
	return _nodeType;
}

DOMNodeReference DOMNode::parentNode() const
{
	return _parentNode;
}

DOMNodeListReference DOMNode::childNodes() const
{
	return _childNodes;
}

DOMNodeReference DOMNode::firstChild() const
{
	if (_childNodes->length() == 0) {
		return DOMNodeReference();
	}
	return _childNodes->item(0);
}

DOMNodeReference DOMNode::lastChild() const
{
	if (_childNodes->length() == 0) {
		return DOMNodeReference();
	}
	return _childNodes->item(_childNodes->length() - 1);
}

DOMNodeReference DOMNode::previousSibling() const
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

DOMNodeReference DOMNode::nextSibling() const
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

DOMDocumentReference DOMNode::ownerDocument() const
{
	return _ownerDocument;
}

DOMNodeReference DOMNode::insertBefore(DOMNodeReference& newChild, DOMNodeReference& refChild)
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

DOMNodeReference DOMNode::replaceChild(DOMNodeReference& newChild, DOMNodeReference& oldChild)
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

DOMNodeReference DOMNode::removeChild(DOMNodeReference& oldChild)
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

DOMNodeReference DOMNode::appendChild(DOMNodeReference& newChild)
{
	// TODO Throw following exceptions mentioned in http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core.html#ID-184E7107:
	// * HIERARCHY_REQUEST_ERR
	// * NO_MODIFICATION_ALLOWED_ERR
	// * NOT_SUPPORTED_ERR
	//if (!isSameNode(DOMNodeReference(newChild->ownerDocument()))) {
	//	throw Exception(DOMError(DOMError::WRONG_DOCUMENT_ERR, SOURCE_LOCATION_ARGS));
	//}
	// TODO
	throw std::wstring(L"Not implemented yet");
}

bool DOMNode::hasChildNodes() const
{
	return (_childNodes->length() > 0);
}

DOMNodeReference DOMNode::cloneNode(bool deep) const
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

void DOMNode::normalize()
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

bool DOMNode::isSupported(const DOMString& feature, const DOMString& version) const
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

unsigned int DOMNode::compareDocumentPosition(DOMNodeReference& other) const
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

DOMString DOMNode::textContent() const
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

void DOMNode::setTextContent(const DOMString& newContent)
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

bool DOMNode::isSameNode(DOMNodeReference& other) const
{
	return (this == other.pointer());
}

DOMString DOMNode::lookupPrefix() const
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

bool DOMNode::isDefaultNamespace(const DOMString& namespaceURI) const
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

DOMString DOMNode::lookupNamespaceURI(const DOMString& prefix) const
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

bool DOMNode::isEqualNode(DOMNodeReference& other) const
{
	// TODO
	throw std::wstring(L"Not implemented yet");
}

} // namespace isl


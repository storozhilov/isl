#ifndef ISL__DOM_NODE__HXX
#define ISL__DOM_NODE__HXX

#include <isl/Reference.hxx>
#include <isl/DOMString.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * DOMNode
 * ---------------------------------------------------------------------------*/

class DOMNodeList;
class DOMNamedNodeMap;
class DOMDocument;

class DOMNode
{
public:
	enum NodeType {
		ELEMENT_NODE = 1,
		ATTRIBUTE_NODE = 2,
		TEXT_NODE = 3,
		CDATA_SECTION_NODE = 4,
		ENTITY_REFERENCE_NODE = 5,
		ENTITY_NODE = 6,
		PROCESSING_INSTRUCTION_NODE = 7,
		COMMENT_NODE = 8,
		DOCUMENT_NODE = 9,
		DOCUMENT_TYPE_NODE = 10,
		DOCUMENT_FRAGMENT_NODE = 11,
		NOTATION_NODE = 12
	};
	enum DocumentPosition {
		DOCUMENT_POSITION_DISCONNECTED = 0x01,
		DOCUMENT_POSITION_PRECEDING = 0x02,
		DOCUMENT_POSITION_FOLLOWING = 0x04,
		DOCUMENT_POSITION_CONTAINS = 0x08,
		DOCUMENT_POSITION_CONTAINED_BY = 0x10,
		DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC = 0x20
	};

	DOMNode(NodeType nodeType, const Reference<DOMDocument>& ownerDocument);
	virtual ~DOMNode();

	Reference<DOMNode> parentNode() const;
	NodeType nodeType() const;
	Reference<DOMNodeList> childNodes() const;
	Reference<DOMNode> firstChild() const;
	Reference<DOMNode> lastChild() const;
	Reference<DOMNode> previousSibling() const;
	Reference<DOMNode> nextSibling() const;
	Reference<DOMDocument> ownerDocument() const;
	Reference<DOMNode> insertBefore(Reference<DOMNode>& newChild, Reference<DOMNode>& refChild);
	Reference<DOMNode> replaceChild(Reference<DOMNode>& newChild, Reference<DOMNode>& oldChild);
	Reference<DOMNode> removeChild(Reference<DOMNode>& oldChild);
	Reference<DOMNode> appendChild(Reference<DOMNode>& newChild);
	bool hasChildNodes() const;
	Reference<DOMNode> cloneNode(bool deep) const;
	void normalize();
	bool isSupported(const DOMString& feature, const DOMString& version) const;
	unsigned int compareDocumentPosition(Reference<DOMNode>& other) const;
	DOMString textContent() const;
	void setTextContent(const DOMString& newContent);
	bool isSameNode(Reference<DOMNode>& other) const;
	DOMString lookupPrefix() const;
	bool isDefaultNamespace(const DOMString& namespaceURI) const;
	DOMString lookupNamespaceURI(const DOMString& prefix) const;
	bool isEqualNode(Reference<DOMNode>& other) const;
	//DOMObject getFeature(const DOMString& feature, const DOMString& version) const;
	//DOMUserData setUserData(const DOMString& key, const DOMUserData& data, const UserDataHandler& handler);
	//DOMUserData getUserData(const DOMString& key) const;

	virtual DOMString nodeName() const = 0;
	virtual DOMString nodeValue() const = 0;
	virtual Reference<DOMNamedNodeMap> attributes() const = 0;
	virtual DOMString namespaceURI() const = 0;
	virtual DOMString prefix() const = 0;
	virtual DOMString localName() const = 0;
private:
	DOMNode();

	NodeType _nodeType;
	Reference<DOMDocument> _ownerDocument;
	Reference<DOMNode> _parentNode;
	Reference<DOMNodeList> _childNodes;
};


typedef Reference<DOMNode> DOMNodeReference;

} // namespace isl

#endif


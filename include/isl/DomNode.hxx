#ifndef ISL__DOM_NODE__HXX
#define ISL__DOM_NODE__HXX

#include <isl/DomString.hxx>
#include <isl/DomNodeList.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * DomNode
 * ---------------------------------------------------------------------------*/

class DomNamedNodeMap;
class DomDocument;

class DomNode
{
public:
	enum NodeType {
		ElementNode = 1,
		AttributeNode = 2,
		TextNode = 3,
		CDataSectionNode = 4,
		EntityReferenceNode = 5,
		EntityNode = 6,
		ProcessingInstructionNode = 7,
		CommentNode = 8,
		DocumentNode = 9,
		DocumentTypeNode = 10,
		DocumentFragmentNode = 11,
		NotationNode = 12
	};
	enum DocumentPosition {
		DocumentPositionDisconnected = 0x01,
		DocumentPositionPreceding = 0x02,
		DocumentPositionFollowing = 0x04,
		DocumentPositionContains = 0x08,
		DocumentPositionContainedBy = 0x10,
		DocumentPositionImplementationSpecific = 0x20
	};

	virtual ~DomNode();

	DomNode * parentNode() const;
	NodeType nodeType() const;
	DomNodeList * childNodes() const;
	DomNode * firstChild() const;
	DomNode * lastChild() const;
	DomNode * previousSibling() const;
	DomNode * nextSibling() const;
	DomDocument * ownerDocument() const;
	DomNode * insertBefore(DomNode * newChild, DomNode * refChild);
	DomNode * replaceChild(DomNode * newChild, DomNode * oldChild);
	DomNode * removeChild(DomNode * oldChild);
	DomNode * appendChild(DomNode * newChild);
	bool hasChildNodes() const;
	DomNode * cloneNode(bool deep) const;
	void normalize();
	bool isSupported(const DomString& feature, const DomString& version) const;
	unsigned int compareDocumentPosition(DomNode * other) const;
	DomString textContent() const;
	void setTextContent(const DomString& newContent);
	bool isSameNode(DomNode * other) const;
	DomString lookupPrefix() const;
	bool isDefaultNamespace(const DomString& namespaceURI) const;
	DomString lookupNamespaceURI(const DomString& prefix) const;
	bool isEqualNode(DomNode * other) const;
	//DomObject getFeature(const DomString& feature, const DomString& version) const;
	//DomUserData setUserData(const DomString& key, const DomUserData& data, const UserDataHandler& handler);
	//DomUserData getUserData(const DomString& key) const;

	virtual DomString nodeName() const = 0;
	virtual DomString nodeValue() const = 0;
	virtual DomNamedNodeMap * attributes() const = 0;
	virtual DomString namespaceURI() const = 0;
	virtual DomString prefix() const = 0;
	virtual DomString localName() const = 0;
protected:
	DomNode(NodeType nodeType, DomDocument * ownerDocument, bool isReadOnly = false);
	bool isReadOnly() const;
	static void getElementsByTagNameRecursive(const DomString& name, DomNode * const parent, DomNodeList& elements);
	static void getElementsByTagNameNSRecursive(const DomString& namespaceURI, const DomString& localName, DomNode * const parent,
			DomNodeList& elements);
private:
	DomNode();

	const NodeType _nodeType;
	DomDocument * _ownerDocument;
	DomNode * _parentNode;
	DomNodeList _childNodes;
	bool _isReadOnly;

	friend class DomNamedNodeMap;
};

} // namespace isl

#endif


#ifndef ISL__DOM_DOCUMENT__HXX
#define ISL__DOM_DOCUMENT__HXX

#include <isl/DOMNode.hxx>
#include <isl/Reference.hxx>

namespace isl
{

class DOMNamedNodeMap;

class DOMDocument : public DOMNode
{
public:
	DOMDocument();

	virtual DOMString nodeName() const;
	virtual DOMString nodeValue() const;
	virtual Reference<DOMNamedNodeMap> attributes() const;
	virtual DOMString namespaceURI() const;
	virtual DOMString prefix() const;
	virtual DOMString localName() const;
private:
};

typedef Reference<DOMDocument> DOMDocumentReference;

} // namespace isl

#endif


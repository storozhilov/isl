#include <isl/DOMDocument.hxx>
#include <isl/DOMNamedNodeMap.hxx>

namespace isl
{

DOMDocument::DOMDocument() :
	DOMNode(DOCUMENT_NODE, DOMDocumentReference())
{}

DOMString DOMDocument::nodeName() const
{
	return DOMString(L"#document");
}

DOMString DOMDocument::nodeValue() const
{
	return DOMString();
}

DOMNamedNodeMapReference DOMDocument::attributes() const
{
	return DOMNamedNodeMapReference();
}

DOMString DOMDocument::namespaceURI() const
{
	return DOMString();
}

DOMString DOMDocument::prefix() const
{
	return DOMString();
}

DOMString DOMDocument::localName() const
{
	return DOMString();
}

} // namespace isl


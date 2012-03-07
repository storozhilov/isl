#include <isl/DomText.hxx>
#include <isl/DomDocument.hxx>
#include <isl/DomError.hxx>
#include <isl/Exception.hxx>

namespace isl
{

DomText::DomText(const DomString& data, bool isElementContentWhitespace, DomDocument * ownerDocument, bool isReadOnly) :
	DomCharacterData(data, DomNode::TextNode, ownerDocument, isReadOnly),
	_isElementContentWhitespace(isElementContentWhitespace)
{}

DomText * DomText::splitText(unsigned int offset)
{
	if (isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	if (offset >= length()) {
		throw Exception(DomError(DomError::IndexSizeErr, SOURCE_LOCATION_ARGS));
	}
	if (!ownerDocument()) {
		return 0;
	}
	DomText * newTextNode = ownerDocument()->createTextNode(substringData(offset, length() - offset));
	deleteData(offset, length() - offset);
	if (parentNode()) {
		parentNode()->insertBefore(newTextNode, nextSibling());
	}
	return newTextNode;
}

bool DomText::isElementContentWhitespace() const
{
	return _isElementContentWhitespace;
}

DomString DomText::wholeText() const
{
	// TODO
	throw Exception(DomError(DomError::MethodNotImplemented, SOURCE_LOCATION_ARGS));
}

DomText * DomText::replaceWholeText(const DomString& content)
{
	// TODO
	throw Exception(DomError(DomError::MethodNotImplemented, SOURCE_LOCATION_ARGS));
}

DomString DomText::nodeName() const
{
	return L"#text";
}

DomString DomText::nodeValue() const
{
	return data();
}

DomNamedNodeMap * DomText::attributes() const
{
	return 0;
}

DomString DomText::namespaceURI() const
{
	return DomString();
}

DomString DomText::prefix() const
{
	return DomString();
}

DomString DomText::localName() const
{
	return DomString();
}

} // namespace isl


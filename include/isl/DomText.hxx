#ifndef ISL__DOM_TEXT__HXX
#define ISL__DOM_TEXT__HXX

#include <isl/DomCharacterData.hxx>

namespace isl
{

class DomText : public DomCharacterData
{
public:
	DomText * splitText(unsigned int offset);
	bool isElementContentWhitespace() const;
	DomString wholeText() const;
	DomText * replaceWholeText(const DomString& content);

	virtual DomString nodeName() const;
	virtual DomString nodeValue() const;
	virtual DomNamedNodeMap * attributes() const;
	virtual DomString namespaceURI() const;
	virtual DomString prefix() const;
	virtual DomString localName() const;
private:
	DomText();
	DomText(const DomText&);
	DomText(const DomString& data, bool isElementContentWhitespace, DomDocument * ownerDocument, bool isReadOnly = false);

	DomText& operator=(const DomText&);

	bool _isElementContentWhitespace;

	friend class DomDocument;
};

} // namespace isl

#endif


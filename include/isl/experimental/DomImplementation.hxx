#ifndef ISL__DOM_IMPLEMENTATION__HXX
#define ISL__DOM_IMPLEMENTATION__HXX

#include <isl/AbstractDomImplementation.hxx>

namespace isl
{

class DomDocument;
class DomDocumentType;

class DomImplementation : public AbstractDomImplementation
{
public:
	DomImplementation();

	//DOMDocumentType createDocumentType(const DomString& qualifiedName, const DomString& publicId,
	//		const DomString& systemId) throw Exception const;
	DomDocument * createDocument(const DomString& namespaceURI = DomString(), const DomString& qualifiedName = DomString(),
			DomDocumentType * doctype = 0) const;

	virtual bool hasFeature(const DomString& feature, const DomString& version) const;
	//virtual DomObject getFeature(const DomString& feature, const DomString& version) const;
};

} // namespace isl

#endif


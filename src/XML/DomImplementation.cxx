#include <isl/DomImplementation.hxx>
#include <isl/DomDocument.hxx>
#include <isl/DomDocumentType.hxx>

namespace isl
{

DomImplementation::DomImplementation() :
	AbstractDomImplementation()
{}

DomDocument * DomImplementation::createDocument(const DomString& namespaceURI, const DomString& qualifiedName,
		DomDocumentType * doctype) const
{
	return new DomDocument(namespaceURI, qualifiedName, const_cast<DomImplementation * const>(this));
}

bool DomImplementation::hasFeature(const DomString& feature, const DomString& version) const
{
	return ((feature == L"Core") || (feature == L"XML")) && (version == L"3.0");
}

} // namespace isl


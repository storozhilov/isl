#ifndef ISL__DOM_IMPLEMENTATION__HXX
#define ISL__DOM_IMPLEMENTATION__HXX

#include <isl/Reference.hxx>
#include <isl/DOMString.hxx>
#include <isl/Exception.hxx>

namespace isl
{

// TODO
class DOMImplementation
{
public:
	DOMImplementation();

	//bool hasFeature(const DOMStringReference feature, const DOMStringReference version) const;
	//DOMDocumentTypeReference createDocumentType(const DOMStringReference qualifiedName, const DOMStringReference publicId,
	//		const DOMStringReference systemId) throw Exception const;
	//DOMDocumentReference createDocument(const DOMStringReference namespaceURI, const DOMStringReference qualifiedName,
	//		const DOMDocumentTypeReference doctype) throw Exception const;
	//DOMObjectReference getFeature(const DOMStringReference feature, const DOMStringReference version) const;
private:
};

typedef Reference<DOMImplementation> DOMImplementationReference;

} // namespace isl

#endif


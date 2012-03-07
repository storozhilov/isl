#ifndef ISL__DOM_DOCUMENT_TYPE__HXX
#define ISL__DOM_DOCUMENT_TYPE__HXX

#include <isl/DomString.hxx>

namespace isl
{

class DomDocumentType
{
public:
private:
	DomDocumentType();
	DomDocumentType(const DomDocumentType&);

	DomDocumentType& operator=(const DomDocumentType&);

	friend class DomImplementation;
};

} // namespace isl

#endif


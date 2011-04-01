#ifndef ISL__DOM_IMPLEMENTATION_LS__HXX
#define ISL__DOM_IMPLEMENTATION_LS__HXX

#include <isl/AbstractDomImplementation.hxx>

namespace isl
{

class DomImplementationLS : public AbstractDomImplementation
{
public:
	DomImplementationLS();

	virtual bool hasFeature(const DomString& feature, const DomString& version) const;
	//virtual DomObject getFeature(const DomString& feature, const DomString& version) const;
};

} // namespace isl

#endif


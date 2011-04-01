#ifndef ISL__ABSTRACT_DOM_IMPLEMENTATION__HXX
#define ISL__ABSTRACT_DOM_IMPLEMENTATION__HXX

#include <isl/DomString.hxx>

namespace isl
{

class AbstractDomImplementation
{
public:
	AbstractDomImplementation();
	virtual ~AbstractDomImplementation();

	virtual bool hasFeature(const DomString& feature, const DomString& version) const = 0;
	//virtual DomObject getFeature(const DomString& feature, const DomString& version) const = 0;
};

} // namespace isl

#endif


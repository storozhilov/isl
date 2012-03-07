#include <isl/DomImplementationLS.hxx>

namespace isl
{

DomImplementationLS::DomImplementationLS() :
	AbstractDomImplementation()
{}

bool DomImplementationLS::hasFeature(const DomString& feature, const DomString& version) const
{
	return ((feature == L"LS") || (feature == L"LS-Async")) && (version == L"3.0");
}

} // namespace isl


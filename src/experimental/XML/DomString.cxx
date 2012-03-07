#include <isl/DomString.hxx>

namespace isl
{

DomString::DomString() :
	std::wstring()
{}

DomString::DomString(const std::wstring& str) :
	std::wstring(str)
{}

DomString::DomString(const std::wstring& str, size_t pos, size_t n) :
	std::wstring(str, pos, n)
{}

DomString::DomString(const wchar_t * s, size_t n) :
	std::wstring(s, n)
{}
DomString::DomString(const wchar_t * s) :
	std::wstring(s)
{}

DomString::DomString(size_t n, wchar_t c) :
	std::wstring(n, c)
{}

bool DomString::isNull() const
{
	return empty();
}

DomString DomString::parsePrefix() const
{
	size_t colonPos = find(L':');
	if (colonPos == std::wstring::npos) {
		return DomString();
	}
	return substr(0, colonPos);
}

DomString DomString::parseLocalName() const
{
	size_t colonPos = find(L':');
	if (colonPos == std::wstring::npos) {
		return *this;
	}
	return substr(colonPos + 1, length() - colonPos - 1);
}

} // namespace isl


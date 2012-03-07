#ifndef ISL__DOM_STRING__HXX
#define ISL__DOM_STRING__HXX

#include <string>

namespace isl
{

class DomString : public std::wstring
{
public:
	DomString();
	DomString(const std::wstring& str);
	DomString(const std::wstring& str, size_t pos, size_t n = std::wstring::npos);
	DomString(const wchar_t * s, size_t n);
	DomString(const wchar_t * s);
	DomString(size_t n, wchar_t c);
	template <typename InputIterator> DomString(InputIterator begin, InputIterator end) :
		std::wstring(begin, end)
	{}

	DomString parsePrefix() const;
	DomString parseLocalName() const;

	bool isNull() const;
};

} // namespace isl

#endif


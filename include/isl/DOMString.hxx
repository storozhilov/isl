#ifndef ISL__DOM_STRING__HXX
#define ISL__DOM_STRING__HXX

#include <string>

namespace isl
{

class DOMString : public std::wstring
{
public:
	DOMString();
	DOMString(const std::wstring& str);
	DOMString(const std::wstring& str, size_t pos, size_t n = std::wstring::npos);
	DOMString(const wchar_t * s, size_t n);
	DOMString(const wchar_t * s);
	DOMString(size_t n, wchar_t c);
	template <typename InputIterator> DOMString(InputIterator begin, InputIterator end) :
		std::wstring(begin, end)
	{}

	bool isNull() const;
};

} // namespace isl

#endif


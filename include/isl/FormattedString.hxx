#ifndef ISL__FORMATTED_STRING__HXX
#define ISL__FORMATTED_STRING__HXX

#include <isl/AbstractFormattedString.hxx>
#include <ctype.h>

namespace isl
{

template <typename Sb, typename Ch, Ch formatSpecifier> class BasicFormattedString : public AbstractFormattedString<Ch>
{
public:
	typedef std::basic_string<Ch> (Sb::* SubstituteMethod)(Ch, const std::basic_string<Ch>&);

	BasicFormattedString(Sb& substitutor, SubstituteMethod method, const std::basic_string<Ch>& format = std::basic_string<Ch>()) :
		AbstractFormattedString<Ch>(format),
		_substitutor(substitutor),
		_method(method)
	{}
private:
	BasicFormattedString();

	virtual typename AbstractFormattedString<Ch>::TokenPosition findToken(size_t pos = 0) const
	{
		typename AbstractFormattedString<Ch>::TokenPosition result;
		result.second = 0;
		// Finding format specifier:
		if (pos >= (AbstractFormattedString<Ch>::_format.length() - 1)) {
			result.first = std::basic_string<Ch>::npos;
			return result;
		}
		result.first = AbstractFormattedString<Ch>::_format.find(formatSpecifier, pos);
		if (result.first == std::basic_string<Ch>::npos) {
			return result;
		}
		if (result.first >= (AbstractFormattedString<Ch>::_format.length() - 1)) {
			result.first = std::basic_string<Ch>::npos;
			return result;
		}

		result.second = 2;
		// Returning "%%" if found
		if (AbstractFormattedString<Ch>::_format[result.first + result.second - 1] == formatSpecifier) {
			return result;
		}
		// Finding token trailing alfa character
		while ((result.first + result.second) <= AbstractFormattedString<Ch>::_format.length()) {
			if (isalpha(AbstractFormattedString<Ch>::_format[result.first + result.second - 1])) {
				return result;
			}
			if (isspace(AbstractFormattedString<Ch>::_format[result.first + result.second - 1])) {
				break;
			}
			result.second++;
		}
		result.first = std::basic_string<Ch>::npos;
		result.second = 0;
		return result;
	}

	virtual std::basic_string<Ch> substitute(const typename AbstractFormattedString<Ch>::TokenPosition& tokenPosition) const
	{
		// Returning "%" instead if "%%"
		if ((tokenPosition.second == 2) && (AbstractFormattedString<Ch>::_format[tokenPosition.first + 1] == formatSpecifier)) {
			return std::basic_string<Ch>(1, formatSpecifier);
		}
		// Calling the method by it's pointer
		return (_substitutor.*_method)(AbstractFormattedString<Ch>::_format[tokenPosition.first + tokenPosition.second - 1],
				AbstractFormattedString<Ch>::_format.substr(tokenPosition.first + 1, tokenPosition.second - 2));
	}

	Sb& _substitutor;
	SubstituteMethod _method;
};

// C++ does not allow templated typedefs so we have to use the inheritance here:

template <typename Sb> class FormattedWString : public BasicFormattedString<Sb, wchar_t, L'%'>
{
public:
	FormattedWString(Sb& substitutor, typename BasicFormattedString<Sb, wchar_t, L'%'>::SubstituteMethod method,
			const std::basic_string<wchar_t>& format = std::basic_string<wchar_t>()) :
		BasicFormattedString<Sb, wchar_t, L'%'>(substitutor, method, format)
	{}
private:
	FormattedWString();
};

// C++ does not allow templated typedefs so we have to use the inheritance here:

template <typename Sb> class FormattedString : public BasicFormattedString<Sb, char, '%'>
{
public:
	FormattedString(Sb& substitutor, typename BasicFormattedString<Sb, char, '%'>::SubstituteMethod method,
			const std::basic_string<char>& format = std::basic_string<char>()) :
		BasicFormattedString<Sb, char, '%'>(substitutor, method, format)
	{}
private:
	FormattedString();
};

} // namespace isl

#endif

#ifndef ISL__ABSTRACT_DATE_TIME_FORMATTER__HXX
#define ISL__ABSTRACT_DATE_TIME_FORMATTER__HXX

#include <isl/AbstractFormatter.hxx>
#include <isl/String.hxx>

namespace isl
{

template <typename Ch> class AbstractDateTimeFormatter : public AbstractFormatter<Ch>
{
public:
	AbstractDateTimeFormatter(const std::basic_string<Ch>& format, Ch tokenSpecifier = '%') :
		AbstractFormatter<Ch>(format),
		_tokenSpecifier(tokenSpecifier)
	{}
protected:
	virtual std::basic_string<Ch> substitute(Ch tokenSymbol) const = 0;
private:
	AbstractDateTimeFormatter();

	virtual typename isl::AbstractFormatter<Ch>::TokenPosition findToken(size_t startPosition = 0) const
	{
		typename isl::AbstractFormatter<Ch>::TokenPosition result(startPosition, 0);
		while (true) {
			if (result.first >= (isl::AbstractFormatter<Ch>::_format.length() - 1)) {
				result.first = std::basic_string<Ch>::npos;
				return result;
			}
			// Finding format specifier:
			result.first = isl::AbstractFormatter<Ch>::_format.find(_tokenSpecifier, result.first);
			if (result.first == std::basic_string<Ch>::npos) {
				return result;
			}
			if (result.first >= (isl::AbstractFormatter<Ch>::_format.length() - 1)) {
				result.first = std::basic_string<Ch>::npos;
				return result;
			}
			// Parsing token
			result.second = 2;
			if ((isl::AbstractFormatter<Ch>::_format[result.first + result.second - 1] == _tokenSpecifier) ||
					String::isChar(isl::AbstractFormatter<Ch>::_format[result.first + result.second - 1])) {
				// Token found -> return its positions
				return result;
			} else {
				// Not a token -> lookup further
				++result.first;
			}
		}
	}
	virtual std::basic_string<Ch> substituteToken(const std::basic_string<Ch>& token) const
	{
		if (token[1] == _tokenSpecifier) {
			// Returning "<token_specifier>" if "<token_specifier><token_specifier>" token has been provided
			return std::basic_string<Ch>(1, _tokenSpecifier);
		}
		return substitute(token[1]);
	}

	Ch _tokenSpecifier;
};

} // namespace isl

#endif

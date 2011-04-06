#ifndef ISL__FORMAT__HXX
#define ISL__FORMAT__HXX

#include <isl/AbstractFormat.hxx>
#include <isl/Variant.hxx>
#include <vector>

namespace isl
{
//! Format stucture: $[{<format_argumants>}]<argument_number>
template <typename Ch> class BasicFormat : public AbstractFormat<Ch>
{
public:
	BasicFormat(const std::basic_string<Ch>& format = std::basic_string<Ch>(), Ch formatSpecifier = '$') :
		AbstractFormat<Ch>(format),
		_formatSpecifier(formatSpecifier),
		_arguments(),
		_curArgNo(0),
		_curParams()
	{}

	typename isl::BasicFormat<Ch>& appendArgument(const Variant& argValue)
	{
		_arguments.push_back(argValue);
		return *this;
	}
	//! Alias for appendArgument
	inline typename isl::BasicFormat<Ch>& arg(const Variant& argValue)
	{
		return appendArgument(argValue);
	}
	typename isl::BasicFormat<Ch>& resetArguments()
	{
		_arguments.clear();
		return *this;
	}
protected:
	virtual std::basic_string<Ch> substitute(unsigned int argNo, const std::basic_string<Ch>& params) const
	{
		if (argNo >= _arguments.size()) {
			return std::basic_string<Ch>();
		}
		return std::basic_string<Ch>(1, '&');
	}
private:
	BasicFormat();

/*	virtual typename isl::AbstractFormat<Ch>::TokenPosition findToken(size_t startPosition = 0) const
	{
		typename isl::AbstractFormat<Ch>::TokenPosition result;
		result.second = 0;
		// Finding format specifier:
		if (startPosition >= (isl::AbstractFormat<Ch>::_format.length() - 1)) {
			result.first = std::basic_string<Ch>::npos;
			return result;
		}
		result.first = isl::AbstractFormat<Ch>::_format.find(_formatSpecifier, startPosition);
		if (result.first == std::basic_string<Ch>::npos) {
			return result;
		}
		if (result.first >= (isl::AbstractFormat<Ch>::_format.length() - 1)) {
			result.first = std::basic_string<Ch>::npos;
			return result;
		}

		result.second = 2;
		// Returning "<specifier><specifier>" if found
		if (isl::AbstractFormat<Ch>::_format[result.first + result.second - 1] == _formatSpecifier) {
			return result;
		}
		// Finding token trailing alfa character
		while ((result.first + result.second) <= isl::AbstractFormat<Ch>::_format.length()) {
			if (isalpha(isl::AbstractFormat<Ch>::_format[result.first + result.second - 1])) {
				return result;
			}
			if (isspace(isl::AbstractFormat<Ch>::_format[result.first + result.second - 1])) {
				break;
			}
			result.second++;
		}
		result.first = std::basic_string<Ch>::npos;
		result.second = 0;
		return result;
	}*/
	virtual typename isl::AbstractFormat<Ch>::TokenPosition findToken(size_t startPosition = 0) const
	{
		typename isl::AbstractFormat<Ch>::TokenPosition result(startPosition, 0);
		while (true) {
			if (result.first >= (isl::AbstractFormat<Ch>::_format.length() - 1)) {
				result.first = std::basic_string<Ch>::npos;
				return result;
			}
			// Finding format specifier:
			result.first = isl::AbstractFormat<Ch>::_format.find(_formatSpecifier, result.first);
			if (result.first == std::basic_string<Ch>::npos) {
				return result;
			}
			if (result.first >= (isl::AbstractFormat<Ch>::_format.length() - 1)) {
				result.first = std::basic_string<Ch>::npos;
				return result;
			}
			// Parsing token
			result.second = 2;
			if (isl::AbstractFormat<Ch>::_format[result.first + result.second - 1] == _formatSpecifier) {
				// Returning "<specifier><specifier>" if found
				return result;
			} else if ((isl::AbstractFormat<Ch>::_format[result.first + result.second - 1] >= '0') && (isl::AbstractFormat<Ch>::_format[result.first + result.second - 1] <= '9')) {
				// Paramless token
				_curArgNo = isl::AbstractFormat<Ch>::_format[result.first + result.second - 1] - '0';
				_curParams = std::basic_string<Ch>();
				return result;
			} else if (isl::AbstractFormat<Ch>::_format[result.first + result.second - 1] == '{') {
				// TODO Parsing format parameters
				throw std::runtime_error("Parameterized format does not implemented yet (TODO)");
			} else {
				// Not a token -> lookup further
				++result.first;
			}
		}
	}
	virtual std::basic_string<Ch> substituteToken(const std::basic_string<Ch>& token) const
	{

		std::cout << "Token to substitute: '" << token << "'" << std::endl;
		if ((token.length() == 2) && (token[0] == _formatSpecifier) && (token[1] == _formatSpecifier)) {
			// Returning "<specifier>" instead if "<specifier><specifier>%"
			return std::basic_string<Ch>(1, _formatSpecifier);
		}
		return substitute(_curArgNo, _curParams);
	}

	typedef std::vector<Variant> ArgumentsList;

	Ch _formatSpecifier;
	ArgumentsList _arguments;
	mutable unsigned int _curArgNo;
	mutable std::basic_string<Ch> _curParams;
};

typedef BasicFormat<char> Format;
typedef BasicFormat<wchar_t> WFormat;

} // namespace isl

#endif

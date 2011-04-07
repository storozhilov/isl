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
	//! Alias for isl::BasicFormat<Ch>& appendArgument(const Variant&)
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
		return _arguments[argNo].format(params);
	}
private:
	BasicFormat();

	inline bool isParamNoChar(Ch ch) const
	{
		return ((ch >= '0') && (ch <= '9')) || ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z'));
	}
	inline unsigned int paramNoByChar(Ch ch) const
	{
		if ((ch >= '0') && (ch <= '9')) {
			return ch - '0';
		} else if ((ch >= 'a') && (ch <= 'z')) {
			return ch - 'a' + 10;
		} else {
			return ch - 'A' + 10;
		}
	}

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
			} else if (isParamNoChar(isl::AbstractFormat<Ch>::_format[result.first + result.second - 1])) {
				_curArgNo = paramNoByChar(isl::AbstractFormat<Ch>::_format[result.first + result.second - 1]);
				_curParams = std::basic_string<Ch>();
				return result;
			} else if (isl::AbstractFormat<Ch>::_format[result.first + result.second - 1] == '{') {
				// Parsing format parameters
				int internalLevelsCount = 0;
				while (true) {
					++result.second;
					if ((result.first + result.second) > isl::AbstractFormat<Ch>::_format.length()) {
						result.first = std::basic_string<Ch>::npos;
						return result;
					}
					if (isl::AbstractFormat<Ch>::_format[result.first + result.second - 1] == '{') {
						++internalLevelsCount;
					} else if ((isl::AbstractFormat<Ch>::_format[result.first + result.second - 1] == '}')) {
						if (internalLevelsCount <= 0) {
							if ((result.first + result.second + 1) > isl::AbstractFormat<Ch>::_format.length()) {
								result.first = std::basic_string<Ch>::npos;
								return result;
							}
							if (isParamNoChar(isl::AbstractFormat<Ch>::_format[result.first + result.second])) {
								++result.second;
								_curParams = isl::AbstractFormat<Ch>::_format.substr(result.first + 2, result.second - 4);
								_curArgNo = paramNoByChar(isl::AbstractFormat<Ch>::_format[result.first + result.second - 1]);
								return result;
							} else {
								result.first += (result.second + 1);
								break;
							}
						} else {
							--internalLevelsCount;
						}
					}
				}
			} else {
				// Not a token -> lookup further
				++result.first;
			}
		}
	}
	virtual std::basic_string<Ch> substituteToken(const std::basic_string<Ch>& token) const
	{

		if ((token.length() == 2) && (token[0] == _formatSpecifier) && (token[1] == _formatSpecifier)) {
			// Returning "<specifier>" instead if "<specifier><specifier>"
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

template <> std::basic_string<char> BasicFormat<char>::substitute(unsigned int argNo, const std::basic_string<char>& params) const
{
	if (argNo >= _arguments.size()) {
		return std::basic_string<char>();
	}
	return Utf8TextCodec().encode(_arguments[argNo].format(Utf8TextCodec().decode(params)));
}

typedef BasicFormat<char> Format;
typedef BasicFormat<wchar_t> WFormat;

} // namespace isl

#endif

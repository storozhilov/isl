#ifndef ISL__VARIANT_FORMATTER__HXX
#define ISL__VARIANT_FORMATTER__HXX

#include <isl/AbstractFormatter.hxx>
#include <isl/Variant.hxx>
#include <isl/String.hxx>
#include <vector>

namespace isl
{
//! Base class for Variant objects formatting
/*!
  Format token stucture:
    - <token_specifier><token_symbol>
    - <token_specifier>{<token_parameters>}<token_symbol>
*/
template <typename Ch> class BasicVariantFormatter : public AbstractFormatter<Ch>
{
public:
	//! Constructs format object
	/*!
	  \param format Format string
	  \param tokenSpecifier Token specifier character (default is '$')
	*/	
	BasicVariantFormatter(const std::basic_string<Ch>& format, Ch tokenSpecifier = '$') :
		AbstractFormatter<Ch>(format),
		_tokenSpecifier(tokenSpecifier),
		_arguments(),
		_curFormatSymbol(),
		_curParams()
	{}
	BasicVariantFormatter() :
		AbstractFormatter<Ch>(),
		_tokenSpecifier('$'),
		_arguments(),
		_curFormatSymbol(),
		_curParams()
	{}

	//! Appends argument to the format
	/*!
	  \param argValue Argument value
	  \return Reference to the format object
	*/
	typename isl::BasicVariantFormatter<Ch>& appendArgument(const Variant& argValue)
	{
		_arguments.push_back(argValue);
		return *this;
	}
	//! Appends argument to the format
	/*!
	  Alias for isl::BasicVariantFormatter<Ch>& appendArgument(const Variant&)
	*/
	inline typename isl::BasicVariantFormatter<Ch>& arg(const Variant& argValue)
	{
		return appendArgument(argValue);
	}
	//! Clears format arguments
	typename isl::BasicVariantFormatter<Ch>& resetArguments()
	{
		_arguments.clear();
		return *this;
	}
protected:
	//! Returns string to substitute format token
	/*!
	  Override this method for custom bevaviour.
	  \param tokenSymbol Token symbol
	  \param tokenParams Format parameters
	  \return String to substitute
	*/
	virtual std::basic_string<Ch> substitute(Ch tokenSymbol, const std::basic_string<Ch>& tokenParams = std::basic_string<Ch>()) const;
private:

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
			if (isl::AbstractFormatter<Ch>::_format[result.first + result.second - 1] == _tokenSpecifier) {
				// Returning "<specifier><specifier>" if found
				return result;
			} else if (isParamNoChar(isl::AbstractFormatter<Ch>::_format[result.first + result.second - 1])) {
				_curFormatSymbol = isl::AbstractFormatter<Ch>::_format[result.first + result.second - 1];
				_curParams = std::basic_string<Ch>();
				return result;
			} else if (isl::AbstractFormatter<Ch>::_format[result.first + result.second - 1] == '{') {
				// Parsing format parameters
				int internalLevelsCount = 0;
				while (true) {
					++result.second;
					if ((result.first + result.second) > isl::AbstractFormatter<Ch>::_format.length()) {
						result.first = std::basic_string<Ch>::npos;
						return result;
					}
					if (isl::AbstractFormatter<Ch>::_format[result.first + result.second - 1] == '{') {
						++internalLevelsCount;
					} else if ((isl::AbstractFormatter<Ch>::_format[result.first + result.second - 1] == '}')) {
						if (internalLevelsCount <= 0) {
							if ((result.first + result.second + 1) > isl::AbstractFormatter<Ch>::_format.length()) {
								result.first = std::basic_string<Ch>::npos;
								return result;
							}
							if (isParamNoChar(isl::AbstractFormatter<Ch>::_format[result.first + result.second])) {
								++result.second;
								_curParams = isl::AbstractFormatter<Ch>::_format.substr(result.first + 2, result.second - 4);
								_curFormatSymbol = isl::AbstractFormatter<Ch>::_format[result.first + result.second - 1];
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

		if ((token.length() == 2) && (token[0] == _tokenSpecifier) && (token[1] == _tokenSpecifier)) {
			// Returning "<token_specifier>" if "<token_specifier><token_specifier>" token has been provided
			return std::basic_string<Ch>(1, _tokenSpecifier);
		}
		return substitute(_curFormatSymbol, _curParams);
	}

	typedef std::vector<Variant> ArgumentsList;

	Ch _tokenSpecifier;
	ArgumentsList _arguments;
	mutable Ch _curFormatSymbol;
	mutable std::basic_string<Ch> _curParams;
};

//! Method specialization for one byte character strings
template <> std::basic_string<char> BasicVariantFormatter<char>::substitute(char tokenSymbol, const std::basic_string<char>& tokenParams) const
{
	unsigned int argNo = paramNoByChar(tokenSymbol);
	if (argNo >= _arguments.size()) {
		return std::basic_string<char>();
	}
	return String::utf8Encode(_arguments[argNo].format(String::utf8Decode(tokenParams)));
}

//! Method specialization for one wide character strings
template <> std::basic_string<wchar_t> BasicVariantFormatter<wchar_t>::substitute(wchar_t tokenSymbol, const std::basic_string<wchar_t>& tokenParams) const
{
	unsigned int argNo = paramNoByChar(tokenSymbol);
	if (argNo >= _arguments.size()) {
		return std::basic_string<wchar_t>();
	}
	return _arguments[argNo].format(tokenParams);
}

typedef BasicVariantFormatter<char> VariantFormatter;			//! For narrow character strings
typedef BasicVariantFormatter<wchar_t> VariantWFormatter;		//! For wide character strings

} // namespace isl

#endif

#ifndef ISL__ABSTRACT_FORMATTED_STRING__HXX
#define ISL__ABSTRACT_FORMATTED_STRING__HXX

#include <string>

namespace isl
{

template <typename Ch> class AbstractFormattedString
{
public:
	AbstractFormattedString(const std::basic_string<Ch>& format = std::basic_string<Ch>()) :
		_format(format)
	{}
	virtual ~AbstractFormattedString()
	{}

	std::basic_string<Ch> format() const
	{
		return _format;
	}

	void setFormat(const std::basic_string<Ch>& newFormat)
	{
		_format = newFormat;
	}
	//std::basic_string<Ch> string() const
	std::basic_string<Ch> str() const
	{
		size_t startPosition = 0;
		std::basic_string<Ch> result;
		do {
			// Finding token
			//TokenPosition tokenPosition = findToken(startPosition);
			std::pair<size_t, size_t> tokenPosition = findToken(startPosition);
			// Appending to the result characters before token if found or the rest of _format otherwise
			result.append(_format.substr(startPosition, tokenPosition.first - startPosition));
			// Exiting if the token is not found
			if (tokenPosition.first == std::basic_string<Ch>::npos) {
				break;
			}
			// Appending to the result token replacement
			result.append(substitute(tokenPosition));
			// Updating start position
			startPosition = tokenPosition.first + tokenPosition.second;
			// Exiting if the token was at the end of the _format
			if (startPosition >= _format.length()) {
				break;
			}
		} while (true);
		return result;
	}
protected:
	typedef std::pair<size_t, size_t> TokenPosition;		// <token_start_position, token_length>

	virtual TokenPosition findToken(size_t pos = 0) const = 0;
	virtual std::basic_string<Ch> substitute(const TokenPosition& tokenPosition) const = 0;

	std::basic_string<Ch> _format;
private:
	AbstractFormattedString();
};

} // namespace isl

#endif

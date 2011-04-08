#ifndef ISL__ABSTRACT_FORMAT__HXX
#define ISL__ABSTRACT_FORMAT__HXX

#include <string>

namespace isl
{

//! Abstract class for string formatting
template <typename Ch> class AbstractFormat
{
protected:
	typedef std::pair<size_t, size_t> TokenPosition;		// <token_start_position, token_length>
public:
	AbstractFormat(const std::basic_string<Ch>& format = std::basic_string<Ch>()) :
		_format(format)
	{}
	virtual ~AbstractFormat()
	{}

	std::basic_string<Ch> format() const
	{
		return _format;
	}
	void setFormat(const std::basic_string<Ch>& newFormat)
	{
		_format = newFormat;
	}
	std::basic_string<Ch> compose() const
	{
		size_t startPosition = 0;
		std::basic_string<Ch> result;
		do {
			// Finding token
			TokenPosition tokenPosition = findToken(startPosition);
			// Appending to the result characters before token if found or the rest of _format otherwise
			result.append(_format.substr(startPosition, tokenPosition.first - startPosition));
			// Exiting if the token is not found
			if (tokenPosition.first == std::basic_string<Ch>::npos) {
				break;
			}
			// Appending to the result token replacement
			result.append(substituteToken(_format.substr(tokenPosition.first, tokenPosition.second)));
			// Updating start position
			startPosition = tokenPosition.first + tokenPosition.second;
		} while (startPosition < _format.length());
		return result;
	}
protected:
	virtual TokenPosition findToken(size_t startPosition = 0) const = 0;
	virtual std::basic_string<Ch> substituteToken(const std::basic_string<Ch>& token) const = 0;

	std::basic_string<Ch> _format;
private:
	AbstractFormat();
};

} // namespace isl

#endif

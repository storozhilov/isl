#ifndef ISL__ABSTRACT_FORMAT__HXX
#define ISL__ABSTRACT_FORMAT__HXX

#include <string>

namespace isl
{

//! Abstract class for string formatting
/*!
  In order to implement your custom format you should override findToken() and substituteToken() methods
*/
template <typename Ch> class AbstractFormat
{
protected:
	//! Token position: <token_start_position, token_length>
	typedef std::pair<size_t, size_t> TokenPosition;
public:
	//! Constructs abstract format object
	/*!
	  \param format Format string
	*/
	AbstractFormat(const std::basic_string<Ch>& format = std::basic_string<Ch>()) :
		_format(format)
	{}
	virtual ~AbstractFormat()
	{}
	
	//! Returns format string
	std::basic_string<Ch> format() const
	{
		return _format;
	}
	//! Sets format string
	/*!
	  \param newFormat New format string value
	*/
	void setFormat(const std::basic_string<Ch>& newFormat)
	{
		_format = newFormat;
	}
	//! Composes format
	/*!
	  \return Composed format
	*/
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
	//! Searches for token
	/*!
	  \param startPosition From where to search
	  \return Token position
	*/
	virtual TokenPosition findToken(size_t startPosition = 0) const = 0;
	//! Returns token substitution
	/*!
	  \param token Token to substitute
	  \return Token substitution
	*/
	virtual std::basic_string<Ch> substituteToken(const std::basic_string<Ch>& token) const = 0;

	std::basic_string<Ch> _format;
//private:
//	AbstractFormat();
};

} // namespace isl

#endif

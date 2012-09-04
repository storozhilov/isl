#ifndef ISL__CHAR__HXX
#define ISL__CHAR__HXX

#include <ctype.h>

namespace isl
{

//! Character type hepler routines class
class Char
{
public:
	//! Inspects if the character is space
	/*!
	  \param ch Character to inspect
	*/
	static inline bool isSpace(unsigned char ch)
	{
		return (ch == 0x20);
	}
	//! Inspects if the character is tab
	/*!
	  \param ch Character to inspect
	*/
	static inline bool isTab(unsigned char ch)
	{
		return (ch == 0x09);
	}
	//! Inspects if the character is space or tab
	/*!
	  \param ch Character to inspect
	*/
	static inline bool isSpaceOrTab(unsigned char ch)
	{
		return (isSpace(ch) || isTab(ch));
	}
	//! Inspects if the character is carriage return
	/*!
	  \param ch Character to inspect
	*/
	static inline bool isCarriageReturn(unsigned char ch)
	{
		return (ch == 0x0D);
	}
	//! Inspects if the character is line feed
	/*!
	  \param ch Character to inspect
	*/
	static inline bool isLineFeed(unsigned char ch)
	{
		return (ch == 0x0A);
	}
	//! Inspects if the character is digit
	/*!
	  \param ch Character to inspect
	*/
	static inline bool isDigit(unsigned char ch)
	{
		return isdigit(ch);
	}
	//! Inspects if the character is hex digit
	/*!
	  \param ch Character to inspect
	*/
	static inline bool isHexDigit(unsigned char ch)
	{
		return isxdigit(ch);
	}
	//! Inspects if the character is URL-safe
	/*!
	  \param ch Character to inspect
	*/
	static inline bool isUrlSafe(unsigned char ch)
	{
		return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || isDigit(ch) || (ch == '_');
	}
	//! Returns the value of the hex digit
	/*!
	  \param ch Hex digit
	*/
	static unsigned char hexValue(unsigned char ch)
	{
		if (isDigit(ch)) {
			return (ch - '0');
		} else if (ch >= 'a' && ch <= 'f') {
			return (ch - 'a' + 10);
		} else if (ch >= 'A' && ch <= 'F') {
			return (ch - 'A' + 10);
		} else {
			return 0;
		}
	}
};

} // namespace isl

#endif

#ifndef ISL__CHAR__HXX
#define ISL__CHAR__HXX

#include <ctype.h>

namespace isl
{

//! Character type hepler routines class
class Char
{
public:
	static inline bool isSpace(unsigned char ch)
	{
		return (ch == 0x20);
	}
	static inline bool isTab(unsigned char ch)
	{
		return (ch == 0x09);
	}
	static inline bool isSpaceOrTab(unsigned char ch)
	{
		return (isSpace(ch) || isTab(ch));
	}
	static inline bool isCarriageReturn(unsigned char ch)
	{
		return (ch == 0x0D);
	}
	static inline bool isLineFeed(unsigned char ch)
	{
		return (ch == 0x0A);
	}
	static inline bool isDigit(unsigned char ch)
	{
		return isdigit(ch);
	}
	static inline bool isHexDigit(unsigned char ch)
	{
		return isxdigit(ch);
	}
	static inline bool isUrlSafe(unsigned char ch)
	{
		return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || isDigit(ch) || (ch == '_');
	}
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

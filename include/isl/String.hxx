#ifndef ISL__STRING__HXX
#define ISL__STRING__HXX

#include <string>

namespace isl
{

//! String functions helper class with all static members only
class String
{
public:
	// Character functions (TODO Move them to Char class?)
	static bool isChar(unsigned char ch);
	static bool isAlpha(unsigned char ch);
	static bool isAlpha(unsigned wchar_t ch);
	static bool isUpperAlpha(unsigned char ch);
	static bool isLowerAlpha(unsigned char ch);
	static bool isDigit(unsigned char ch);
	static bool isHexDigit(unsigned char ch);
	static bool isControl(unsigned char ch);
	static bool isCarriageReturn(unsigned char ch);
	static bool isLineFeed(unsigned char ch);
	static bool isSpace(unsigned char ch);
	static bool isTab(unsigned char ch);
	static bool isSpaceOrTab(unsigned char ch);
	static bool isSeparator(unsigned char ch);
	static bool isToken(unsigned char ch);
	static bool isUrlSafe(unsigned char ch);
	static unsigned char hexValue(unsigned char ch);

	// String functions
	//! Trims space characters on the both ends of the string
	static void trim(std::string &str);
	//! Trims space characters on the both ends of the string and returns the result
	static std::string trim(const std::string &str);
	//! Encodes string using URL-encoding
	static std::string urlEncode(const std::string &str);
	//! Decodes string using URL-encoding
	static std::string urlDecode(const std::string &str);
	//! Replaces all occurences of the substring with the another one
	static void replaceAll(std::string &str, const std::string& find, const std::string& replace);
	//! Encodes wchar_t to char using 'plain' encoding
	static std::wstring asciiToWString(const std::string& str);
	//! Decodes char to  wchar_t using 'plain' encoding
	static std::string wstringToAscii(const std::wstring& str);
	//! Encodes wchar_t to char using UTF-8 encoding
	static void utf8Encode(std::string& dest, const wchar_t * source, size_t size);
	//! Encodes wchar_t to char using UTF-8 encoding
	static void utf8Encode(std::string& dest, const wchar_t * source);
	//! Encodes wchar_t to char using UTF-8 encoding
	static void utf8Encode(std::string& dest, const std::wstring& source);
	//! Encodes wchar_t to char using UTF-8 encoding
	static std::string utf8Encode(const wchar_t * source, size_t size);
	//! Encodes wchar_t to char using UTF-8 encoding
	static std::string utf8Encode(const wchar_t * source);
	//! Encodes wchar_t to char using UTF-8 encoding
	static std::string utf8Encode(const std::wstring& source);
	//! Decodes char to wchar_t using UTF-8 encoding
	static void utf8Decode(std::wstring& dest, const char * source, size_t size);
	//! Decodes char to wchar_t using UTF-8 encoding
	static void utf8Decode(std::wstring& dest, const char * source);
	//! Decodes char to wchar_t using UTF-8 encoding
	static void utf8Decode(std::wstring& dest, const std::string& source);
	//! Decodes char to wchar_t using UTF-8 encoding
	static std::wstring utf8Decode(const char * source, size_t size);
	//! Decodes char to wchar_t using UTF-8 encoding
	static std::wstring utf8Decode(const char * source);
	//! Decodes char to wchar_t using UTF-8 encoding
	static std::wstring utf8Decode(const std::string& source);
};

} // namespace isl

#endif

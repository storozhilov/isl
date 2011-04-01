#ifndef ISL__STRING__HXX
#define ISL__STRING__HXX

#include <string>

namespace isl
{

class String
{
public:
	// Character functions
	static bool isChar(unsigned char ch);
	static bool isAlpha(unsigned char ch);
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
	static void trim(std::string &str);
	static std::string trim(const std::string &str);
	static std::string urlEncode(const std::string &str);
	static std::string urlDecode(const std::string &str);
	static void replace(std::string &str, const std::string& find, const std::string& replace);
	// Converting functions
	static std::wstring asciiToWString(const std::string& str);
	static std::string wstringToAscii(const std::wstring& str);
	/*static void utf8ToWString(std::wstring& dest, const char * source, int size = -1);
	static void utf8ToWString(std::wstring& dest, const std::string& source);
	static std::wstring utf8ToWString(const char * str, int size = -1);
	static std::wstring utf8ToWString(const std::string& str);
	static void wstringToUtf8(std::string& dest, const wchar_t * source, size_t size);
	static void wstringToUtf8(std::string& dest, const wchar_t * source);
	static void wstringToUtf8(std::string& dest, const std::wstring& source);
	static std::string wstringToUtf8(const std::wstring& str);*/
};

} // namespace isl

#endif

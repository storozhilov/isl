#ifndef ISL__STRING__HXX
#define ISL__STRING__HXX

#include <string>

namespace isl
{

//! String functions helper class with all static members only
class String
{
public:
	//! Trims space characters on the both ends of the string
	static void trim(std::string &str);
	//! Trims space characters on the both ends of the string and returns the result
	static std::string trim(const std::string &str);
	//! Encodes string using Percent-encoding (see http://en.wikipedia.org/wiki/Percent-encoding)
	static std::string encodePercent(const std::string &str);
	//! Decodes string using Percent-encoding (see http://en.wikipedia.org/wiki/Percent-encoding)
	static std::string decodePercent(const std::string &str);
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

	enum Base {
		DecimalBase,
		HexBase
	};
	// Conversion functions
	static unsigned int toUnsignedInt(const std::string& str, bool * errorOccured = 0, Base base = DecimalBase);
};

} // namespace isl

#endif

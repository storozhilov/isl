#include <isl/String.hxx>
#include <isl/Char.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <sstream>
#include <iomanip>
#include <string.h>

namespace isl
{

void String::trim(std::string &str)
{
	std::string charsToTrim(" \t\r\n");
	std::string::size_type pos = str.find_last_not_of(charsToTrim);
	if (pos == std::string::npos) {
		str.clear();
		return;
	}
	str.erase(pos + 1);
	pos = str.find_first_not_of(charsToTrim);
	str.erase(0, pos);
}

std::string String::trim(const std::string &str)
{
	std::string s(str);
	trim(s);
	return s;
}

std::string String::encodePercent(const std::string &str)
{
	std::ostringstream encodedString;
	encodedString.setf(std::ios::uppercase);
	encodedString.setf(std::ios::hex, std::ios::basefield);
	encodedString.unsetf(std::ios::showbase);
	encodedString.fill('0');
	for (size_t i = 0; i < str.length(); ++i) {
		unsigned char code = str[i];
		if (Char::isSpace(code)) {
			encodedString << '+';
		} else if (!Char::isUrlSafe(code)) {
			encodedString << '%' << std::setw(2) << static_cast<unsigned int>(code);
		} else {
			encodedString << code;
		}
	}
	return encodedString.str();
}

std::string String::decodePercent(const std::string &str)
{
	std::string decodedString;
	size_t i = 0;
	while (i < str.length()) {
		if (str[i] == '%') {
			if ((i + 2) >= str.length() || !Char::isHexDigit(str[i + 1]) || !Char::isHexDigit(str[i + 2])) {
				decodedString += str[i];
				++i;
			} else {
				unsigned char charCode = Char::hexValue(str[i + 1]) * 16 + Char::hexValue(str[i + 2]);
				decodedString += charCode;
				i += 3;
			}
		} else if (str[i] == '+') {
			decodedString += ' ';
			++i;
		} else {
			decodedString += str[i];
			++i;
		}
	}
	return decodedString;
}

void String::replaceAll(std::string &str, const std::string& find, const std::string& replace)
{
	size_t pos;
	while ((pos = str.find(find)) != std::string::npos) {
		str.replace(pos, find.length(), replace);
	}
}

std::wstring String::asciiToWString(const std::string& str)
{
	std::wostringstream oss;
	oss << str.c_str();
	return oss.str();
}

std::string String::wstringToAscii(const std::wstring& str)
{
	std::ostringstream oss;
	oss << str.c_str();
	return oss.str();
}

void String::utf8Encode(std::string& dest, const wchar_t * source, size_t size)
{
	dest.clear();
	if (!source) {
		return;
	}
	dest.reserve(size);							// Best case
	size_t pos = 0;
	// Skip utf8-encoded byte order mark if exists
	if ((size >= 3) && (static_cast<unsigned char>(source[0]) == 0xEF) && (static_cast<unsigned char>(source[1]) == 0xBB) &&
			(static_cast<unsigned char>(source[2]) == 0xBF)) {
		pos += 3;
	}
	wchar_t errorChar = 0xFFFD;
	wchar_t destChar = 0;
	int extraBytes = 0;
	for (; pos < size; ++pos) {
		unsigned char sourceChar = static_cast<unsigned char>(source[pos]);
		if (sourceChar <= 0x7F) {					// Single byte (00000000-01111111: 0xxxxxxx)
			if (extraBytes > 0) {
				dest.push_back(errorChar);
				extraBytes = 0;
			}
			dest.push_back(static_cast<wchar_t>(sourceChar));
		} else if (sourceChar <= 0xBF) {				// Second/third/... byte of sequence (10000000-10111111: 10xxxxxx)
			if (extraBytes > 0) {
				destChar = ((destChar << 6) | (sourceChar & 0x3F));
				--extraBytes;
				if (extraBytes <= 0) {
					dest.push_back(destChar);
				}
			} else {
				dest.push_back(errorChar);
			}
		} else if (sourceChar <= 0xDF) {				// 2-byte sequence start (11000000-11011111: 110xxxxx)
			if (extraBytes > 0) {
				dest.push_back(errorChar);
				extraBytes = 0;
			} else {
				extraBytes = 1;
				destChar = static_cast<wchar_t>(sourceChar & 0x1F);
			}
		} else if (sourceChar <= 0xEF) {				// 3-byte sequence start (11100000-11101111: 1110xxxx)
			if (extraBytes > 0) {
				dest.push_back(errorChar);
				extraBytes = 0;
			} else {
				extraBytes = 2;
				destChar = static_cast<wchar_t>(sourceChar & 0x0F);
			}
		} else if (sourceChar <= 0xF7) {				// 4-byte sequence start (11110000-11110111: 11110xxx)
			if (extraBytes > 0) {
				dest.push_back(errorChar);
				extraBytes = 0;
			} else {
				extraBytes = 3;
				destChar = static_cast<wchar_t>(sourceChar & 0x07);
			}
		} else if (sourceChar <= 0xFB) {				// 5-byte sequence start (11111000-11111011: 111110xx)
			if (extraBytes > 0) {
				dest.push_back(errorChar);
				extraBytes = 0;
			} else {
				extraBytes = 4;
				destChar = static_cast<wchar_t>(sourceChar & 0x03);
			}
		} else if (sourceChar <= 0xFD) {				// 6-byte sequence start (11111100-11111101: 1111110x)
			if (extraBytes > 0) {
				dest.push_back(errorChar);
				extraBytes = 0;
			} else {
				extraBytes = 5;
				destChar = static_cast<wchar_t>(sourceChar & 0x01);
			}
		} else {
			dest.push_back(errorChar);
			extraBytes = 0;
		}
	}
	if (extraBytes > 0) {
		dest.push_back(errorChar);
	}
}

void String::utf8Encode(std::string& dest, const wchar_t * source)
{
	utf8Encode(dest, source, (source) ? wcslen(source) : 0);
}

void String::utf8Encode(std::string& dest, const std::wstring& source)
{
	utf8Encode(dest, source.data(), source.size());
}

std::string String::utf8Encode(const wchar_t * source, size_t size)
{
	std::string dest;
	utf8Encode(dest, source, size);
	return dest;
}

std::string String::utf8Encode(const wchar_t * source)
{
	std::string dest;
	utf8Encode(dest, source, (source) ? wcslen(source) : 0);
	return dest;
}

std::string String::utf8Encode(const std::wstring& source)
{
	std::string dest;
	utf8Encode(dest, source.data(), source.length());
	return dest;
}

void String::utf8Decode(std::wstring& dest, const char * source, size_t size)
{
	dest.clear();
	if (!source) {
		return;
	}
	for (size_t i = 0; i < size; ++i) {
		wchar_t sourceChar = source[i];
		if (sourceChar <= 0x0000007F) {
			dest.push_back(static_cast<char>(sourceChar));
		} else if (sourceChar <= 0x000007FF) {
			dest.push_back(static_cast<char>(0xC0 | ((sourceChar >> 6) & 0x1F)));
			dest.push_back(static_cast<char>(0x80 | (sourceChar & 0x3F)));
		} else if (sourceChar <= 0x0000FFFF) {
			dest.push_back(static_cast<char>(0xE0 | ((sourceChar >> 12) & 0x0F)));
			dest.push_back(static_cast<char>(0x80 | ((sourceChar >> 6) & 0x3F)));
			dest.push_back(static_cast<char>(0x80 | (sourceChar & 0x3F)));
		} else if (sourceChar <= 0x001FFFFF) {
			dest.push_back(static_cast<char>(0xF0 | ((sourceChar >> 18) & 0x07)));
			dest.push_back(static_cast<char>(0x80 | ((sourceChar >> 12) & 0x3F)));
			dest.push_back(static_cast<char>(0x80 | ((sourceChar >> 6) & 0x3F)));
			dest.push_back(static_cast<char>(0x80 | (sourceChar & 0x3F)));
		} else if (sourceChar <= 0x03FFFFFF) {
			dest.push_back(static_cast<char>(0xF8 | ((sourceChar >> 24) & 0x03)));
			dest.push_back(static_cast<char>(0x80 | ((sourceChar >> 18) & 0x3F)));
			dest.push_back(static_cast<char>(0x80 | ((sourceChar >> 12) & 0x3F)));
			dest.push_back(static_cast<char>(0x80 | ((sourceChar >> 6) & 0x3F)));
			dest.push_back(static_cast<char>(0x80 | (sourceChar & 0x3F)));
		} else if (sourceChar <= 0x7FFFFFFF) {
			dest.push_back(static_cast<char>(0xFC | ((sourceChar >> 30) & 0x01)));
			dest.push_back(static_cast<char>(0x80 | ((sourceChar >> 24) & 0x3F)));
			dest.push_back(static_cast<char>(0x80 | ((sourceChar >> 18) & 0x3F)));
			dest.push_back(static_cast<char>(0x80 | ((sourceChar >> 12) & 0x3F)));
			dest.push_back(static_cast<char>(0x80 | ((sourceChar >> 6) & 0x3F)));
			dest.push_back(static_cast<char>(0x80 | (sourceChar & 0x3F)));
		} else {
			dest.push_back('?');
		}
	}
}

void String::utf8Decode(std::wstring& dest, const char * source)
{
	utf8Decode(dest, source, (source) ? strlen(source) : 0);
}

void String::utf8Decode(std::wstring& dest, const std::string& source)
{
	utf8Decode(dest, source.data(), source.size());
}

std::wstring String::utf8Decode(const char * source, size_t size)
{
	std::wstring dest;
	utf8Decode(dest, source, size);
	return dest;
}

std::wstring String::utf8Decode(const char * source)
{
	std::wstring dest;
	utf8Decode(dest, source, (source) ? strlen(source) : 0);
	return dest;
}

std::wstring String::utf8Decode(const std::string& source)
{
	std::wstring dest;
	utf8Decode(dest, source.data(), source.length());
	return dest;
}

unsigned int String::toUnsignedInt(const std::string& str, bool * errorOccured, Base base)
{
	if (errorOccured) {
		*errorOccured = false;
	}
	std::string strToParse = str;
	trim(strToParse);
	if (strToParse.empty()) {
		return 0;
	}
	unsigned int result = 0;
	switch (base) {
		case DecimalBase:
			for (size_t curPos = (strToParse[0] == '+' ? 1 : 0); curPos < strToParse.size(); ++curPos) {
				if (!Char::isDigit(strToParse[curPos])) {
					if (errorOccured) {
						*errorOccured = true;
					}
					return 0;
				}
				unsigned int newResult = result * 10 + strToParse[curPos] - '0';
				if (newResult < result) {
					// Integer overflow has been detected
					if (errorOccured) {
						*errorOccured = true;
					}
					return 0;
				}
				result = newResult;
			}
			break;
		case HexBase:
			for (size_t curPos = 0; curPos < strToParse.size(); ++curPos) {
				if (!Char::isHexDigit(strToParse[curPos])) {
					if (errorOccured) {
						*errorOccured = true;
					}
					return 0;
				}
				unsigned int newResult;
				if (strToParse[curPos] >= '0' && strToParse[curPos] <= '9') {
					newResult = result * 16 + strToParse[curPos] - '0';
				} else if (strToParse[curPos] >= 'a' && strToParse[curPos] <= 'f') {
					newResult = result * 16 + strToParse[curPos] - 'a' + 10;
				} else if (strToParse[curPos] >= 'A' && strToParse[curPos] <= 'F') {
					newResult = result * 16 + strToParse[curPos] - 'A' + 10;
				} else {
					throw Exception(Error(SOURCE_LOCATION_ARGS, "Invalid hex value"));
				}
				if (newResult < result) {
					// Integer overflow has been detected
					if (errorOccured) {
						*errorOccured = true;
					}
					return 0;
				}
				result = newResult;
			}
			break;
		default:
			throw Exception(Error(SOURCE_LOCATION_ARGS, "Invalid base"));
	}
	return result;
}

} // namespace isl

#include <isl/Utf8TextCodec.hxx>

namespace isl
{

Utf8TextCodec::Utf8TextCodec() :
	AbstractTextCodec()
{}

void Utf8TextCodec::encodeText(std::string& dest, const wchar_t * source, size_t size) const
{
	dest.clear();
	if (!source) {
		return;
	}
	dest.reserve(size);							// Worst case
	int pos = 0;
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

void Utf8TextCodec::decodeText(std::wstring& dest, const char * source, size_t size) const
{
	dest.clear();
	if (!source) {
		return;
	}
	for (int i = 0; i < size; ++i) {
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

} // namespace isl


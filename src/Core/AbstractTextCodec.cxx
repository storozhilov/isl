#include <isl/AbstractTextCodec.hxx>
#include <wchar.h>
#include <string.h>

namespace isl
{

AbstractTextCodec::AbstractTextCodec()
{}

AbstractTextCodec::~AbstractTextCodec()
{}

void AbstractTextCodec::encode(std::string& dest, const wchar_t * source, size_t size) const
{
	encodeText(dest, source, size);
}

void AbstractTextCodec::encode(std::string& dest, const wchar_t * source) const
{
	encodeText(dest, source, (source) ? wcslen(source) : 0);
}

void AbstractTextCodec::encode(std::string& dest, const std::wstring& source) const
{
	encodeText(dest, source.data(), source.size());
}

std::string AbstractTextCodec::encode(const wchar_t * source, size_t size) const
{
	std::string dest;
	encodeText(dest, source, size);
	return dest;
}

std::string AbstractTextCodec::encode(const wchar_t * source) const
{
	std::string dest;
	encodeText(dest, source, (source) ? wcslen(source) : 0);
	return dest;
}

std::string AbstractTextCodec::encode(const std::wstring& source) const
{
	std::string dest;
	encodeText(dest, source.data(), source.length());
	return dest;
}

void AbstractTextCodec::decode(std::wstring& dest, const char * source, size_t size) const
{
	decodeText(dest, source, size);
}

void AbstractTextCodec::decode(std::wstring& dest, const char * source) const
{
	decodeText(dest, source, (source) ? strlen(source) : 0);
}

void AbstractTextCodec::decode(std::wstring& dest, const std::string& source) const
{
	decodeText(dest, source.data(), source.size());
}

std::wstring AbstractTextCodec::decode(const char * source, size_t size) const
{
	std::wstring dest;
	decodeText(dest, source, size);
	return dest;
}

std::wstring AbstractTextCodec::decode(const char * source) const
{
	std::wstring dest;
	decodeText(dest, source, (source) ? strlen(source) : 0);
	return dest;
}

std::wstring AbstractTextCodec::decode(const std::string& source) const
{
	std::wstring dest;
	decodeText(dest, source.data(), source.length());
	return dest;
}

} // namespace isl


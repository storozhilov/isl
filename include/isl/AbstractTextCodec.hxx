#ifndef ISL__ABSTRACT_TEXT_CODEC__HXX
#define ISL__ABSTRACT_TEXT_CODEC__HXX

#include <string>

namespace isl
{

class AbstractTextCodec
{
public:
	AbstractTextCodec();
	virtual ~AbstractTextCodec();

	void encode(std::string& dest, const wchar_t * source, size_t size) const;
	void encode(std::string& dest, const wchar_t * source) const;
	void encode(std::string& dest, const std::wstring& source) const;
	std::string encode(const wchar_t * source, size_t size) const;
	std::string encode(const wchar_t * source) const;
	std::string encode(const std::wstring& source) const;
	void decode(std::wstring& dest, const char * source, size_t size) const;
	void decode(std::wstring& dest, const char * source) const;
	void decode(std::wstring& dest, const std::string& source) const;
	std::wstring decode(const char * source, size_t size) const;
	std::wstring decode(const char * source) const;
	std::wstring decode(const std::string& source) const;
protected:
	virtual void encodeText(std::string& dest, const wchar_t * source, size_t size) const = 0;
	virtual void decodeText(std::wstring& dest, const char * source, size_t size) const = 0;
};

} // namespace isl

#endif


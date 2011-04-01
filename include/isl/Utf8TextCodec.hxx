#ifndef ISL__UTF8_TEXT_CODEC__HXX
#define ISL__UTF8_TEXT_CODEC__HXX

#include <isl/AbstractTextCodec.hxx>

namespace isl
{

class Utf8TextCodec : public AbstractTextCodec
{
public:
	Utf8TextCodec();

	virtual void encodeText(std::string& dest, const wchar_t * source, size_t size) const;
	virtual void decodeText(std::wstring& dest, const char * source, size_t size) const;
};

} // namespace isl

#endif


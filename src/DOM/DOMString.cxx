#include <isl/DOMString.hxx>
//#include <isl/String.hxx>
//#include <memory>

namespace isl
{

DOMString::DOMString() :
	std::wstring()
{}

DOMString::DOMString(const std::wstring& str) :
	std::wstring(str)
{}

DOMString::DOMString(const std::wstring& str, size_t pos, size_t n) :
	std::wstring(str, pos, n)
{}

DOMString::DOMString(const wchar_t * s, size_t n) :
	std::wstring(s, n)
{}
DOMString::DOMString(const wchar_t * s) :
	std::wstring(s)
{}

DOMString::DOMString(size_t n, wchar_t c) :
	std::wstring(n, c)
{}

bool DOMString::isNull() const
{
	return empty();
}

/*DOMString::DOMString(const char * const str) :
	_xmlString(XMLString::transcode(str))
{}

DOMString::DOMString(const wchar_t * const str) :
	_xmlString(XMLString::transcode(String::wstringToUtf8(str).c_str()))
{
}

//DOMString::DOMString(XMLCh * str) :
//	_xmlString(str)
//{}

DOMString::~DOMString()
{
	XMLString::release(&_xmlString);
}

bool DOMString::isNull() const
{
	return (!_xmlString);
}

const XMLCh * DOMString::xmlString() const
{
	return _xmlString;
}

std::string DOMString::stdString() const
{
	if (isNull()) {
		return std::string();
	}
	std::auto_ptr<char> buf(XMLString::transcode(_xmlString));
	return std::string(buf.get());
}

std::wstring DOMString::stdWString() const
{
	if (isNull()) {
		return std::wstring();
	}
	std::auto_ptr<char> buf(XMLString::transcode(_xmlString));
	return String::utf8ToWString(buf.get());
}

std::wstring DOMString::stdWString(const XMLCh * const str)
{
	if (!str) {
		return std::wstring();
	}
	std::auto_ptr<char> buf(XMLString::transcode(str));
	return String::utf8ToWString(buf.get());
}*/

} // namespace isl


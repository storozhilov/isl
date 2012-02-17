#include <isl/Http.hxx>
#include <iomanip>

namespace isl
{

Log Http::errorLog;
Log Http::warningLog;
Log Http::debugLog;
Log Http::accessLog;

std::string Http::encodeUrl(const std::string &url)
{
	std::ostringstream encodedString;
	encodedString.setf(std::ios::uppercase);
	encodedString.setf(std::ios::hex, std::ios::basefield);
	encodedString.unsetf(std::ios::showbase);
	encodedString.fill('0');
	for (int i = 0; i < url.length(); ++i) {
		unsigned char code = url[i];
		if (String::isSpace(code)) {
			encodedString << '+';
		} else if (!String::isUrlSafe(code)) {
			encodedString << '%' << std::setw(2) << static_cast<unsigned int>(code);
		} else {
			encodedString << code;
		}
	}
	return encodedString.str();
}

std::string Http::decodeUrl(const std::string &encodedUrl)
{
	std::string decodedString;
	int i = 0;
	while (i < encodedUrl.length()) {
		if (encodedUrl[i] == '%') {
			if ((i + 2) >= encodedUrl.length() || !String::isHexDigit(encodedUrl[i + 1]) || !String::isHexDigit(encodedUrl[i + 2])) {
				decodedString += encodedUrl[i];
				++i;
			} else {
				unsigned char charCode = String::hexValue(encodedUrl[i + 1]) * 16 + String::hexValue(encodedUrl[i + 2]);
				decodedString += charCode;
				i += 3;
			}
		} else if (encodedUrl[i] == '+') {
			decodedString += ' ';
			++i;
		} else {
			decodedString += encodedUrl[i];
			++i;
		}
	}
	return decodedString;
}

Http::Uri Http::parseUri(const std::string& uriStr)
{}

std::string Http::composeUri(const Http::Uri& uri)
{}

Http::Get Http::parseGet(const std::string& getStr)
{}

std::string Http::composeGet(const Http::Get& get)
{}

Http::Post Http::parsePost(const std::string& postStr)
{}

std::string Http::composePost(const Http::Post& get)
{
}

} // namespace isl

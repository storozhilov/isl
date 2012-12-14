#include <isl/Http.hxx>
#include <isl/String.hxx>
#include <isl/HttpRequestCookieParser.hxx>
#include <iomanip>

namespace isl
{

const char * Http::DateTimeFormat = "%a, %d %b %Y %H:%M:%S %Z";
const wchar_t * Http::DateTimeWFormat = L"%a, %d %b %Y %H:%M:%S %Z";

void Http::parseUri(const std::string& uriStr, std::string& path, std::string& query)
{
	size_t questionMarkPos = uriStr.find('?');
	if (questionMarkPos == std::string::npos) {
		path = uriStr;
		query.clear();
	} else {
		path = uriStr.substr(0, questionMarkPos);
		query = uriStr.substr(questionMarkPos + 1);
	}
}

std::string Http::composeUri(const std::string& path, const std::string& query)
{
	std::string uriStr = path;
	if (!query.empty()) {
		uriStr.append(1, '?');
		uriStr.append(query);
	}
	return uriStr;
}

void Http::parseParams(const std::string& paramsStr, Params& params)
{
	params.clear();
	size_t pos = 0;
	while (pos < paramsStr.length()) {
		// Parsing param name
		std::string paramName;
		while (pos < paramsStr.length()) {
			if (paramsStr[pos] == '=') {
				++pos;
				break;
			} else if (paramsStr[pos] == '&') {
				break;
			}
			paramName.append(1, paramsStr[pos]);
			++pos;
		}
		// Parsing param value
		std::string paramValue;
		while (pos < paramsStr.length()) {
			if (paramsStr[pos] == '&') {
				++pos;
				break;
			}
			paramValue.append(1, paramsStr[pos]);
			++pos;
		}
		// Adding name/value pair to the result
		params.insert(Params::value_type(String::decodePercent(paramName), String::decodePercent(paramValue)));
	}
}

std::string Http::composeParams(const Http::Params& params)
{
	std::string paramsStr;
	for (Params::const_iterator i = params.begin(); i != params.end(); ++i) {
		if (i->first.empty()) {
			continue;
		}
		paramsStr.append(String::encodePercent(i->first));
		paramsStr.append(1, '=');
		paramsStr.append(String::encodePercent(i->second));
	}
	return paramsStr;
}

void Http::grabCookies(const Params& header, RequestCookies& cookies)
{
	cookies.clear();
	for (Params::const_iterator i = header.begin(); i != header.end(); ++i) {
		if (i->first != "Cookie") {
			continue;
		}
		HttpRequestCookieParser cookieParser;
		RequestCookies curCookies;
		cookieParser.parse(i->second, curCookies);
		cookies.insert(curCookies.begin(), curCookies.end());
	}
}

void Http::grabCookies(const Params& header, ResponseCookies& cookies)
{
	// TODO
}

} // namespace isl

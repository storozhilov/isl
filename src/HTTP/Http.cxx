#include <isl/Http.hxx>
#include <iomanip>

namespace isl
{

Log Http::errorLog;
Log Http::warningLog;
Log Http::debugLog;
Log Http::accessLog;

void Http::parseUri(const std::string& uriStr, std::string& path, std::string& query)
{
	size_t questionMarkPos = uriStr.find('?');
	path = uriStr.substr(0, questionMarkPos);
	query = uriStr.substr(questionMarkPos + 1);
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
	int pos = 0;
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

} // namespace isl

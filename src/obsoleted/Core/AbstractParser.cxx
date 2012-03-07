#include <isl/AbstractParser.hxx>
#include <string.h>

namespace isl
{

int AbstractParser::parse(const char * str)
{
	return parse(str, strlen(str));
}

int AbstractParser::parse(const std::string str)
{
	return parse(str.data(), str.length());
}

bool AbstractParser::isInTokens(const TokenList& tokens, const std::string& str)
{
	for (TokenList::const_iterator i = tokens.begin(); i != tokens.end(); ++i) {
		if ((*i)->asString() == str) {
			return true;
		}
	}
	return false;
}

AbstractParser::Token AbstractParser::findToken(const TokenList& tokens, const std::string& str)
{
	for (TokenList::const_iterator i = tokens.begin(); i != tokens.end(); ++i) {
		if ((*i)->asString() == str) {
			return Token(*(*i));
		}
	}
	return Token();
}

bool AbstractParser::isFitTokens(const TokenList& tokens, const std::string& str, unsigned char ch)
{
	for (TokenList::const_iterator i = tokens.begin(); i != tokens.end(); ++i) {
		std::string tokenString((*i)->asString());
		if ((tokenString.find(str) == 0) && (tokenString.length() > str.length()) && (tokenString[str.length()] == ch)) {
			return true;
		}
	}
	return false;
}

} // namespace isl

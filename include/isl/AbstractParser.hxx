#ifndef ISL__ABSTRACT_PARSER__HXX
#define ISL__ABSTRACT_PARSER__HXX

#include <isl/NullableEnum.hxx>
#include <string>
#include <list>

#include <iostream>	// To be removed

namespace isl
{

class AbstractParser
{
public:
	class AbstractToken
	{
	public:
		virtual ~AbstractToken() {}
	
		virtual AbstractToken * clone() const = 0;
		virtual std::string asString() const = 0;
	};

	typedef NullableEnum<AbstractToken> Token;
	typedef std::list<AbstractToken *> TokenList;

	int parse(const char * str);								// Returns successfully parsed characters count
	int parse(const std::string str);							// Returns successfully parsed characters count

	virtual int parse(const char * data, unsigned int size) = 0;				// Returns successfully parsed characters count

	static bool isInTokens(const TokenList& tokens, const std::string& str);
	static Token findToken(const TokenList& tokens, const std::string& str);
	static bool isFitTokens(const TokenList& tokens, const std::string& str, unsigned char ch);
};

} // namespace isl

#endif


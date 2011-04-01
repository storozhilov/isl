#ifndef ISL__HTTP_REQUEST_PARSER__HXX
#define ISL__HTTP_REQUEST_PARSER__HXX

#include <isl/AbstractParser.hxx>

namespace isl
{

class HTTPRequest;
	
class HTTPRequestParser : public AbstractParser
{
public:
	// TODO Use isl::Enum or AbstractParser::Token as State
	enum State {
		ParsingRequest,
		ParsingRequestMethod,
		ParsingRequestURI,
		ParsingHTTPVersion,
		ParsingHTTPVersionCRLF,
		ParsingHeaderField,
		ParsingHeaderFieldName,
		ParsingHeaderFieldSeparator,
		ParsingHeaderFieldValue,
		ParsingHeaderFieldValueCRLF,
		ParsingHeaderFieldValueLWS,
		ParsingEndOfHeader,
		ParsingBody,
		ParsingCompleted,
		// Error states
		BadRequest,
		RequestURITooLong,
		RequestHeaderTooLong,
		RequestEntityTooLong,
		MethodNotImplemented,
		HTTPVersionNotImplemented,
		InvalidRequestURI
		// ... TODO
	};

	virtual ~HTTPRequestParser();

	void reset();
	virtual int parse(const char * data, unsigned int size);
	bool needMoreData() const;
	State state() const;
	bool isBadRequest() const;
	bool isCompleteRequest() const;
	bool requestBodyExpected() const;
	bool canBeAddedToRequestURI(unsigned char ch) const;
private:
	HTTPRequestParser();
	HTTPRequestParser(HTTPRequest * request);

	bool parseURI();									// Returns "false" if bad URI
	void parseCookies();

	HTTPRequest * _request;
	TokenList _methods;
	TokenList _methodsImplemented;
	TokenList _versions;
	TokenList _versionsImplemented;
	State _state;
	std::string _requestMethodString;
	std::string _httpVersionString;
	std::string _headerFieldName;
	std::string _headerFieldValue;

	friend class HTTPRequest;
};

} // namespace isl

#endif


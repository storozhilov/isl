#ifndef ISL__HTTP_MESSAGE__HXX
#define ISL__HTTP_MESSAGE__HXX

#include <isl/AbstractParser.hxx>
#include <string>
#include <map>
#include <vector>

namespace isl
{

class AbstractHTTPTask;

class HTTPMessage
{
public:
	HTTPMessage(AbstractHTTPTask * task);
	virtual ~HTTPMessage();

	// HTTP-version tokens
	class HTTP_0_9_Version : public AbstractParser::AbstractToken
	{
	public:
		virtual AbstractParser::AbstractToken * clone() const { return new HTTP_0_9_Version(*this); }
		virtual std::string asString() const { return "HTTP/0.9"; }
	};
	class HTTP_1_0_Version : public AbstractParser::AbstractToken
	{
	public:
		virtual AbstractParser::AbstractToken * clone() const { return new HTTP_1_0_Version(*this); }
		virtual std::string asString() const { return "HTTP/1.0"; }
	};
	class HTTP_1_1_Version : public AbstractParser::AbstractToken
	{
	public:
		virtual AbstractParser::AbstractToken * clone() const { return new HTTP_1_1_Version(*this); }
		virtual std::string asString() const { return "HTTP/1.1"; }
	};

	//AbstractHTTPTask& const task();
	bool headerContains(const std::string &fieldName, const std::string &fieldValue) const;
	std::string headerValue(const std::string &fieldName) const;
	std::list<std::string> headerValues(const std::string &fieldName) const;
	const AbstractParser::Token& version() const;
	void setHeaderField(const std::string &fieldName, const std::string &fieldValue, bool replaceIfExists);
	void setHeaderField(const std::string &fieldName, int fieldValue, bool replaceIfExists);
	void resetHeaderField(const std::string &fieldName);
protected:
	void reset();

	typedef std::multimap<std::string, std::string> Header;
	
	static const unsigned int DefaultTransferTimeout = 100000;				// In milliseconds

	AbstractHTTPTask * _task;
	AbstractParser::Token _version;
	Header _header;
	unsigned int _transferTimeout;								// In milliseconds
private:
	HTTPMessage();
	HTTPMessage(const HTTPMessage&);							// No copy

	HTTPMessage& operator=(const HTTPMessage&);						// No copy

	//AbstractParser::Token _version;

	friend class HTTPRequestParser;
};

} // namespace isl

#endif

#ifndef ISL__HTTP_REQUEST__OBSOLETED__HXX
#define ISL__HTTP_REQUEST__OBSOLETED__HXX

#include <isl/HTTPMessage.hxx>
#include <isl/HTTPRequestParser.hxx>
#include <string>
#include <map>
#include <vector>

namespace isl
{

class HTTPRequest : public HTTPMessage
{
public:
	HTTPRequest(AbstractHTTPTask * task);
	virtual ~HTTPRequest();

	// HTTP-method tokens
	class OPTIONS_HTTP_Method : public AbstractParser::AbstractToken
	{
	public:
		virtual AbstractParser::AbstractToken * clone() const { return new OPTIONS_HTTP_Method(*this); }
		virtual std::string asString() const { return "OPTIONS"; }
	};
	class GET_HTTP_Method : public AbstractParser::AbstractToken
	{
	public:
		virtual AbstractParser::AbstractToken * clone() const { return new GET_HTTP_Method(*this); }
		virtual std::string asString() const { return "GET"; }
	};
	class HEAD_HTTP_Method : public AbstractParser::AbstractToken
	{
	public:
		virtual AbstractParser::AbstractToken * clone() const { return new HEAD_HTTP_Method(*this); }
		virtual std::string asString() const { return "HEAD"; }
	};
	class POST_HTTP_Method : public AbstractParser::AbstractToken
	{
	public:
		virtual AbstractParser::AbstractToken * clone() const { return new POST_HTTP_Method(*this); }
		virtual std::string asString() const { return "POST"; }
	};
	class PUT_HTTP_Method : public AbstractParser::AbstractToken
	{
	public:
		virtual AbstractParser::AbstractToken * clone() const { return new PUT_HTTP_Method(*this); }
		virtual std::string asString() const { return "PUT"; }
	};
	class DELETE_HTTP_Method : public AbstractParser::AbstractToken
	{
	public:
		virtual AbstractParser::AbstractToken * clone() const { return new DELETE_HTTP_Method(*this); }
		virtual std::string asString() const { return "DELETE"; }
	};
	class TRACE_HTTP_Method : public AbstractParser::AbstractToken
	{
	public:
		virtual AbstractParser::AbstractToken * clone() const { return new TRACE_HTTP_Method(*this); }
		virtual std::string asString() const { return "TRACE"; }
	};
	class CONNECT_HTTP_Method : public AbstractParser::AbstractToken
	{
	public:
		virtual AbstractParser::AbstractToken * clone() const { return new CONNECT_HTTP_Method(*this); }
		virtual std::string asString() const { return "CONNECT"; }
	};
	
	AbstractParser::Token method() const;
	std::string uri() const;
	std::string resource() const;
	std::string query() const;
	std::string host() const;
	int port() const;
	const std::map<std::string, std::string>& get();
	const std::map<std::string, std::string>& cookies();
	const std::map<std::string, std::string>& post();
	bool hasCookie(const std::string& cookieName) const;
	std::string cookieValue(const std::string& cookieName) const;
	void receive(bool nextKeepAliveCycle = false);
	bool isBad() const;
	bool isComplete() const;
protected:
private:
	HTTPRequest();

	//static const unsigned int DefaultTimeoutMilliSec = 100000;
	static const unsigned int DefaultKeepAliveTimeout = 10000;				// In milliseconds
	static const unsigned int DefaultMaxURISize = 1024;
	static const unsigned int DefaultMaxHeaderSize = 4096;
	static const unsigned int DefaultMaxSize = 1048576;
	static const unsigned int DefaultBufferSize = 4096;
	
	void reset();

	//unsigned int _timeout;
	unsigned int _keepAliveTimeout;
	unsigned int _maxURISize;
	unsigned int _maxHeaderSize;
	unsigned int _maxSize;
	HTTPRequestParser _parser;
	AbstractParser::Token _method;
	std::string _uri;
	std::string _resource;
	std::string _query;
	std::string _host;
	int _port;
	std::map<std::string, std::string> _get;
	std::map<std::string, std::string> _cookies;
	std::map<std::string, std::string> _post;
	std::vector<char> _transferBuffer;

	friend class HTTPRequestParser;
};

} // namespace isl

#endif


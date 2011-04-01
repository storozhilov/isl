#ifndef ISL__ABSTRACT_HTTP_TASK__NEW__HXX
#define ISL__ABSTRACT_HTTP_TASK__NEW__HXX

#include <isl/AbstractTcpTask.hxx>
#include <isl/Log.hxx>
//#include <isl/HTTPRequest.hxx>
//#include <isl/HTTPResponse.hxx>

namespace isl
{

class AbstractHttpTask : public AbstractTcpTask
{
public:
	AbstractHttpTask(TcpSocket * socket);
	//virtual ~AbstractHttpTask();

	//static Log errorLog;
	//static Log warningLog;
	//static Log debugLog;
	//static Log accessLog;

//	bool connectionToBeClosed() const;
protected:
//	virtual HTTPResponse::AbstractGenerator * createGeneratorOK() = 0;
//	virtual HTTPResponse::AbstractGenerator * createGeneratorBadRequest();
//	virtual HTTPResponse::AbstractGenerator * createGeneratorInternalServerError();
private:
	AbstractHttpTask();
	AbstractHttpTask(const AbstractHttpTask&);					// No copy

	AbstractHttpTask& operator=(const AbstractHttpTask&);				// No copy

	virtual bool methodImplemented(const std::string& method) const;
	virtual bool versionImplemented(const std::string& version) const;
	virtual void executeImplementation(Worker& worker);

	void setMethod(const std::string& method);
	void setUri(const std::string& uri);
	void setVersion(const std::string& version);

//	HTTPRequest _request;
//	HTTPResponse _response;
//	bool _keepAlive;
//	unsigned int _maxKeepAliveRequests;						// 0 - unlimited
//	unsigned int _requestsReceived;

//	static bool DefaultKeepAlive;
//	static unsigned int DefaultMaxKeepAliveRequests;

//	friend class HTTPResponse;
//	friend class HTTPResponse::AbstractGenerator;
};

/*
		static const int	Timeout					= 100;			// In seconds
		static const bool	KeepAlive				= true;
		static const int	KeepAliveTimeout			= 10;			// In seconds
		static const int	MaxKeepAliveRequests			= 100;			// 0 - unlimited
		static const int	MaxRequestURISize			= 1024;			// 1K
		static const int	MaxRequestHeaderSize			= 4096;			// 4K
		static const int	MaxRequestSize				= 1048576;		// 1M
		static const int	BodyChunkSize				= 16384;		// 16K
		static const int	MaxBufferedBodySize			= 1048576;		// 1M
		static const int	MaxResourceGeneratorsCount		= 2;

*/

} // namespace isl

#endif


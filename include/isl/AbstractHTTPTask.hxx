#ifndef ISL__ABSTRACT_HTTP_TASK__HXX
#define ISL__ABSTRACT_HTTP_TASK__HXX

#include <isl/AbstractTcpService.hxx>
#include <isl/Log.hxx>
#include <isl/HTTPRequest.hxx>
#include <isl/HTTPResponse.hxx>

namespace isl
{

class AbstractHTTPTask : public AbstractTcpService::AbstractTask
{
public:
	AbstractHTTPTask(TcpSocket * socket);
	//virtual ~AbstractHTTPTask();

	static Log errorLog;
	static Log warningLog;
	static Log debugLog;
	static Log accessLog;

	bool connectionToBeClosed() const;
protected:
	virtual HTTPResponse::AbstractGenerator * createGeneratorOK() = 0;
	virtual HTTPResponse::AbstractGenerator * createGeneratorBadRequest();
	virtual HTTPResponse::AbstractGenerator * createGeneratorInternalServerError();
private:
	AbstractHTTPTask();
	AbstractHTTPTask(const AbstractHTTPTask&);					// No copy

	AbstractHTTPTask& operator=(const AbstractHTTPTask&);				// No copy

	virtual void executeImplementation(Worker& worker);

	HTTPRequest _request;
	HTTPResponse _response;
	bool _keepAlive;
	unsigned int _maxKeepAliveRequests;						// 0 - unlimited
	unsigned int _requestsReceived;

	static bool DefaultKeepAlive;
	static unsigned int DefaultMaxKeepAliveRequests;

	friend class HTTPResponse;
	friend class HTTPResponse::AbstractGenerator;
};

} // namespace isl

#endif


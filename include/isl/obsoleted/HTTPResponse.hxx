#ifndef ISL__HTTP_RESPONSE__HXX
#define ISL__HTTP_RESPONSE__HXX

#include <isl/HTTPMessage.hxx>
#include <isl/HTTPRequest.hxx>
#include <isl/NullableEnum.hxx>
#include <isl/DateTime.hxx>
#include <list>
#include <sstream>

namespace isl
{

class HTTPRequest;

// TODO Rewrite header and extra header handling

class HTTPResponse : public HTTPMessage
{
public:
	HTTPResponse(AbstractHTTPTask * task);
	virtual ~HTTPResponse();

	// Body chunk of HTTP-message (used in chunked encoding transmission)
	class BodyChunk
	{
	public:
		BodyChunk(unsigned int capacity);
		~BodyChunk();
	
		void clear();
		unsigned int fill(const char *data, unsigned int dataSize);
		unsigned int append(const char *data, unsigned int dataSize);
		bool isFull() const;
		bool isEmpty() const;
		bool attemptedToOverflow() const;
		const char * data() const;
		unsigned int size() const;
		const char * chunkEncodedData() const;
		unsigned int chunkEncodedSize() const;
		const char * identityEncodedData() const;
		unsigned int identityEncodedSize() const;
	private:
		BodyChunk();
	
		void setSize(unsigned int newSize);

		unsigned int _capacity;
		unsigned int _size;
		std::vector<char> _buffer;
		unsigned int _chunkSizeFieldSize;
		unsigned int _chunckedDataStartPos;
		bool _attemptedToOverflow;
	};

	// Status codes (too much of them...):
	class AbstractStatusCode
	{
	public:
		virtual ~AbstractStatusCode() {}
		
		virtual AbstractStatusCode * clone() const = 0;
		virtual int code() const = 0;
		virtual std::string reason() const = 0;

		std::string codeStr() const
		{
			std::ostringstream sstr;
			sstr << code();
			return sstr.str();
		}
	};
	class ContinueStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new ContinueStatusCode(*this); }
		virtual int code() const { return 100; }
		virtual std::string reason() const { return "Continue"; }
	};
	class SwitchingProtocolsStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new SwitchingProtocolsStatusCode(*this); }
		virtual int code() const { return 101; }
		virtual std::string reason() const { return "Switching Protocols"; }
	};
	class OKStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new OKStatusCode(*this); }
		virtual int code() const { return 200; }
		virtual std::string reason() const { return "OK"; }
	};
	class CreatedStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new CreatedStatusCode(*this); }
		virtual int code() const { return 201; }
		virtual std::string reason() const { return "Created"; }
	};
	class AcceptedStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new AcceptedStatusCode(*this); }
		virtual int code() const { return 202; }
		virtual std::string reason() const { return "Accepted"; }
	};
	class NonAuthoritativeInformationStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new NonAuthoritativeInformationStatusCode(*this); }
		virtual int code() const { return 203; }
		virtual std::string reason() const { return "Non-Authoritative Information"; }
	};
	class NoContentStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new NoContentStatusCode(*this); }
		virtual int code() const { return 204; }
		virtual std::string reason() const { return "No Content"; }
	};
	class ResetContentStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new ResetContentStatusCode(*this); }
		virtual int code() const { return 205; }
		virtual std::string reason() const { return "Reset Content"; }
	};
	class PartialContentStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new PartialContentStatusCode(*this); }
		virtual int code() const { return 206; }
		virtual std::string reason() const { return "Partial Content"; }
	};
	class MultipleChoicesStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new MultipleChoicesStatusCode(*this); }
		virtual int code() const { return 300; }
		virtual std::string reason() const { return "Multiple Choices"; }
	};
	class MovedPermanentlyStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new MovedPermanentlyStatusCode(*this); }
		virtual int code() const { return 301; }
		virtual std::string reason() const { return "Moved Permanently"; }
	};
	class FoundStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new FoundStatusCode(*this); }
		virtual int code() const { return 302; }
		virtual std::string reason() const { return "Found"; }
	};
	class SeeOtherStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new SeeOtherStatusCode(*this); }
		virtual int code() const { return 303; }
		virtual std::string reason() const { return "See Other"; }
	};
	class NotModifiedStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new NotModifiedStatusCode(*this); }
		virtual int code() const { return 304; }
		virtual std::string reason() const { return "Not Modified"; }
	};
	class UseProxyStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new UseProxyStatusCode(*this); }
		virtual int code() const { return 305; }
		virtual std::string reason() const { return "Use Proxy"; }
	};
	class TemporaryRedirectStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new TemporaryRedirectStatusCode(*this); }
		virtual int code() const { return 306; }
		virtual std::string reason() const { return "Temporary Redirect"; }
	};
	class BadRequestStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new BadRequestStatusCode(*this); }
		virtual int code() const { return 400; }
		virtual std::string reason() const { return "Bad Request"; }
	};
	class UnauthorizedStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new UnauthorizedStatusCode(*this); }
		virtual int code() const { return 401; }
		virtual std::string reason() const { return "Unauthorized"; }
	};
	class PaymentRequiredStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new PaymentRequiredStatusCode(*this); }
		virtual int code() const { return 402; }
		virtual std::string reason() const { return "Payment Required"; }
	};
	class ForbiddenStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new ForbiddenStatusCode(*this); }
		virtual int code() const { return 403; }
		virtual std::string reason() const { return "Forbidden"; }
	};
	class NotFoundStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new NotFoundStatusCode(*this); }
		virtual int code() const { return 404; }
		virtual std::string reason() const { return "NotFound"; }
	};
	class MethodNotAllowedStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new MethodNotAllowedStatusCode(*this); }
		virtual int code() const { return 405; }
		virtual std::string reason() const { return "Method Not Allowed"; }
	};
	class NotAcceptableStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new NotAcceptableStatusCode(*this); }
		virtual int code() const { return 406; }
		virtual std::string reason() const { return "Not Acceptable"; }
	};
	class ProxyAuthenticationRequiredStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new ProxyAuthenticationRequiredStatusCode(*this); }
		virtual int code() const { return 407; }
		virtual std::string reason() const { return "Proxy Authentication Required"; }
	};
	class RequestTimeOutStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new RequestTimeOutStatusCode(*this); }
		virtual int code() const { return 408; }
		virtual std::string reason() const { return "Request Time-out"; }
	};
	class ConflictStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new ConflictStatusCode(*this); }
		virtual int code() const { return 409; }
		virtual std::string reason() const { return "Conflict"; }
	};
	class GoneStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new GoneStatusCode(*this); }
		virtual int code() const { return 410; }
		virtual std::string reason() const { return "Gone"; }
	};
	class LengthRequiredStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new LengthRequiredStatusCode(*this); }
		virtual int code() const { return 411; }
		virtual std::string reason() const { return "Length Required"; }
	};
	class PreconditionFailedStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new PreconditionFailedStatusCode(*this); }
		virtual int code() const { return 412; }
		virtual std::string reason() const { return "Precondition Failed"; }
	};
	class RequestEntityTooLargeStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new RequestEntityTooLargeStatusCode(*this); }
		virtual int code() const { return 413; }
		virtual std::string reason() const { return "Request Entity Too Large"; }
	};
	class RequestURITooLargeStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new RequestURITooLargeStatusCode(*this); }
		virtual int code() const { return 414; }
		virtual std::string reason() const { return "Request-URI Too Large"; }
	};
	class UnsupportedMediaTypeStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new UnsupportedMediaTypeStatusCode(*this); }
		virtual int code() const { return 415; }
		virtual std::string reason() const { return "Unsupported Media Type"; }
	};
	class RequestedRangeNotSatisfiableStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new RequestedRangeNotSatisfiableStatusCode(*this); }
		virtual int code() const { return 416; }
		virtual std::string reason() const { return "Requested range not satisfiable"; }
	};
	class ExpectationFailedStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new ExpectationFailedStatusCode(*this); }
		virtual int code() const { return 417; }
		virtual std::string reason() const { return "Expectation Failed"; }
	};
	class InternalServerErrorStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new InternalServerErrorStatusCode(*this); }
		virtual int code() const { return 500; }
		virtual std::string reason() const { return "Internal Server Error"; }
	};
	class NotImplementedStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new NotImplementedStatusCode(*this); }
		virtual int code() const { return 501; }
		virtual std::string reason() const { return "Not Implemented"; }
	};
	class BadGatewayStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new BadGatewayStatusCode(*this); }
		virtual int code() const { return 502; }
		virtual std::string reason() const { return "Bad Gateway"; }
	};
	class ServiceUnavailableStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new ServiceUnavailableStatusCode(*this); }
		virtual int code() const { return 503; }
		virtual std::string reason() const { return "Service Unavailable"; }
	};
	class GatewayTimeOutStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new GatewayTimeOutStatusCode(*this); }
		virtual int code() const { return 504; }
		virtual std::string reason() const { return "Gateway Time-out"; }
	};
	class HTTPVersionNotSupportedStatusCode : public AbstractStatusCode
	{
	public:
		virtual AbstractStatusCode * clone() const { return new HTTPVersionNotSupportedStatusCode(*this); }
		virtual int code() const { return 505; }
		virtual std::string reason() const { return "HTTP Version not supported"; }
	};

	typedef NullableEnum<AbstractStatusCode> StatusCode;

	// Response generators
	class AbstractGenerator
	{
	public:
		AbstractGenerator(AbstractHTTPTask * task);
		virtual ~AbstractGenerator();
	
		void generate();
	protected:
		const HTTPRequest& request() const;
		HTTPResponse& response() const;
		AbstractHTTPTask * task() const;

		virtual StatusCode statusCode() const = 0;
		virtual void generateImplementation() = 0;
	private:
		AbstractGenerator();
		AbstractGenerator(const AbstractGenerator&);

		AbstractGenerator& operator=(const AbstractGenerator&);

		AbstractHTTPTask * _task;
	};

	class AbstractGeneratorOK : public AbstractGenerator
	{
	public:
		AbstractGeneratorOK(AbstractHTTPTask * task);
	protected:
		virtual StatusCode statusCode() const;
	private:
		AbstractGeneratorOK();
		AbstractGeneratorOK(const AbstractGeneratorOK&);

		AbstractGeneratorOK& operator=(const AbstractGeneratorOK&);
	};

	class GeneratorBadRequest : public AbstractGenerator
	{
	public:
		GeneratorBadRequest(AbstractHTTPTask * task);
	protected:
		virtual StatusCode statusCode() const;
		virtual void generateImplementation();
	private:
		GeneratorBadRequest();
		GeneratorBadRequest(const GeneratorBadRequest&);

		GeneratorBadRequest& operator=(const GeneratorBadRequest&);
	};

	class GeneratorInternalServerError : public AbstractGenerator
	{
	public:
		GeneratorInternalServerError(AbstractHTTPTask * task);
	protected:
		virtual StatusCode statusCode() const;
		virtual void generateImplementation();
	private:
		GeneratorInternalServerError();
		GeneratorInternalServerError(const GeneratorInternalServerError&);

		GeneratorInternalServerError& operator=(const GeneratorInternalServerError&);
	};

	class AbstractBodyBuffer
	{
	public:
		AbstractBodyBuffer();
		virtual ~AbstractBodyBuffer();

		unsigned int read(char * buffer, unsigned int bufferSize);
		void write(const char * data, unsigned int dataSize);
		void write(const char * str);
		void write(const std::string& str);
		void clear();
		void setFile(const std::string& newFileName);
		void resetFile();
		bool isInFile() const;
		bool isInReadingState() const;
	protected:
		virtual unsigned int readImplementation(char * buffer, unsigned int bufferSize) = 0;
		virtual void writeImplementation(const char * data, unsigned int dataSize) = 0;
		virtual void clearImplementation() = 0;
	private:
		bool _inReadingState;
		std::string _fileName;
	};

	struct Cookie {
		std::string			name;
		std::string			value;
		DateTime			expires;
		std::string			path;
		std::string			domain;
		bool				secure;
	};
	typedef std::list<Cookie> Cookies;

	void generateAndSend();
	void reset();
	AbstractBodyBuffer& inputBuffer();
	AbstractBodyBuffer& outputBuffer();
	StatusCode statusCode() const;
	void setStatusCode(StatusCode newStatusCode);
protected:
private:
	HTTPResponse();

	static const unsigned int DefaultBodyChunkSize = 4096;
	static const char * const DefaultServerSignature;

	class NetworkBodyBuffer : public AbstractBodyBuffer
	{
	public:
		NetworkBodyBuffer(HTTPResponse& response, unsigned int bodyChunkSize);
		//~NetworkBodyBuffer();

		void flush();
		void reset();
		bool transferStarted() const;
	private:
		NetworkBodyBuffer();

		void sendBuffer(const char * buffer, unsigned int size);
		void sendChunk(bool isLastChunk);

		virtual unsigned int readImplementation(char * buffer, unsigned int bufferSize);
		virtual void writeImplementation(const char * data, unsigned int dataSize);
		virtual void clearImplementation();

		HTTPResponse& _response;
		BodyChunk _bodyChunk;
		//bool _defferedTransmission;					// TODO
		bool _transferStarted;
		bool _isChunkedTransferEncoding;
	};

	class NullBodyBuffer : public AbstractBodyBuffer
	{
	public:
		NullBodyBuffer();
		//~NullBodyBuffer();
	private:
		virtual unsigned int readImplementation(char * buffer, unsigned int bufferSize);
		virtual void writeImplementation(const char * data, unsigned int dataSize);
		virtual void clearImplementation();
	};

	typedef std::list<AbstractGenerator *> GeneratorList;

	class GeneratorsResetter
	{
	public:
		GeneratorsResetter(HTTPResponse& response) :
			_response(response)
		{}
		~GeneratorsResetter()
		{
			_response.resetGenerators();
		}
	private:
		GeneratorsResetter();
		GeneratorsResetter(const GeneratorsResetter&);						// No copy

		GeneratorsResetter& operator=(const GeneratorsResetter&);				// No copy
		
		HTTPResponse& _response;
	};

	void resetGenerators();

	Header _extraHeader;
	GeneratorList _generators;
	NullBodyBuffer _sourceBodyBuffer;
	NetworkBodyBuffer _destBodyBuffer;
	StatusCode _statusCode;
	Cookies _cookies;
	std::string _serverSignature;
};

} // namespace isl

#endif


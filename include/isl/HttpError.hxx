#ifndef ISL__HTTP_ERROR__HXX
#define ISL__HTTP_ERROR__HXX

#include <isl/AbstractError.hxx>
#include <isl/Utf8TextCodec.hxx>

namespace isl
{

class HttpError : public AbstractError
{
public:
	class BadRequest : public AbstractType
	{
	public:
		BadRequest(const std::wstring& msg) :
			AbstractType(),
			_msg(msg)
		{}

		virtual AbstractType * clone() const { return new BadRequest(*this); }
		virtual std::wstring message() const { return _msg; }
	private:
		BadRequest();

		std::wstring _msg;
	};
	//class RequestMethodTooLong : public AbstractType
	//{
	//public:
	//	virtual AbstractType * clone() const { return new RequestMethodTooLong(*this); }
	//	virtual std::wstring message() const { return L"Request method is too long"; }
	//};
	class MethodNotImplemented : public AbstractType
	{
	public:
		MethodNotImplemented(const std::string& method) :
			AbstractType(),
			_method(method)
		{}

		virtual AbstractType * clone() const { return new MethodNotImplemented(*this); }
		virtual std::wstring message() const {
			return std::wstring(L"HTTP-method '") + Utf8TextCodec().decode(_method) + L"' is not implemented";
		}
	private:
		MethodNotImplemented();

		std::string _method;
	};
	class RequestUriTooLong : public AbstractType
	{
	public:
		virtual AbstractType * clone() const { return new RequestUriTooLong(*this); }
		virtual std::wstring message() const { return L"Request URI is too long"; }
	};
	class VersionNotImplemented : public AbstractType
	{
	public:
		VersionNotImplemented(const std::string& version) :
			AbstractType(),
			_version(version)
		{}

		virtual AbstractType * clone() const { return new VersionNotImplemented(*this); }
		virtual std::wstring message() const {
			return std::wstring(L"HTTP-version '") + Utf8TextCodec().decode(_version) + L"' is not implemented";
		}
	private:
		VersionNotImplemented();

		std::string _version;
	};
	//class RequestVersionTooLong : public AbstractType
	//{
	//public:
	//	virtual AbstractType * clone() const { return new RequestVersionTooLong(*this); }
	//	virtual std::wstring message() const { return L"Request version is too long"; }
	//};
	//class RequestHeaderFieldNameTooLong : public AbstractType
	//{
	//public:
	//	virtual AbstractType * clone() const { return new RequestHeaderFieldNameTooLong(*this); }
	//	virtual std::wstring message() const { return L"Request header field name is too long"; }
	//};
	class InvalidParserState : public AbstractType
	{
	public:
		InvalidParserState(unsigned int state) :
			AbstractType(),
			_state(state)
		{}

		virtual AbstractType * clone() const { return new InvalidParserState(*this); }
		virtual std::wstring message() const {
			std::wostringstream msg;
			msg << L"Invalid HTTP-request parser state: " << _state;
			return msg.str();
		}
	private:
		InvalidParserState();

		unsigned int _state;
	};

	HttpError(const AbstractType& type, SOURCE_LOCATION_ARGS_DECLARATION) :
		AbstractError(type, SOURCE_LOCATION_ARGS_PASSTHRU)
	{}
	
	virtual AbstractError * clone() const
	{
		return new HttpError(*this);
	}
private:
	HttpError();
};

} // namespace isl

#endif

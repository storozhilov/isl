#ifndef ISL__HTTP_ERROR__HXX
#define ISL__HTTP_ERROR__HXX

#include <isl/AbstractError.hxx>

namespace isl
{

class HttpError : public AbstractInfoError
{
public:
	enum Type {
		BadRequest,
		MethodNotImplemented,
		RequestUriTooLong,
		RequestVersionTooLong,
		VersionNotImplemented,
		InvalidParserState
	};

	HttpError(SOURCE_LOCATION_ARGS_DECLARATION, Type type, const std::wstring& info = std::wstring()) :
		AbstractInfoError(SOURCE_LOCATION_ARGS_PASSTHRU, info),
		_type(type)
	{}

	inline Type type() const
	{
		return _type;
	}

	virtual AbstractError * clone() const
	{
		return new HttpError(*this);
	}
protected:
	virtual std::wstring composeMessage() const
	{
		std::wstring result;
		switch (_type) {
			case BadRequest:
				result = L"Bad request";
				break;
			case MethodNotImplemented:
				result = L"Method not implemented";
				break;
			case RequestUriTooLong:
				result = L"Request URI too long";
				break;
			case RequestVersionTooLong:
				result = L"Request version too long";
				break;
			case VersionNotImplemented:
				result = L"Version not implemented";
				break;
			case InvalidParserState:
				result = L"Invalid HTTP-request parser state";
				break;
			default:
				result = L"Unknown HTTP error";
		}
		appendInfo(result);
		return result;
	}
private:
	HttpError();

	Type _type;
};

} // namespace isl

#endif

#ifndef ISL__IO_ERROR__HXX
#define ISL__IO_ERROR__HXX

#include <isl/AbstractError.hxx>

namespace isl
{

class IOError : public AbstractInfoError
{
public:
	enum Type {
		TimeoutExpired,
		ConnectionAborted,
		DeviceIsNotOpen
	};

	IOError(SOURCE_LOCATION_ARGS_DECLARATION, Type type, const std::wstring& info = std::wstring()) :
		AbstractInfoError(SOURCE_LOCATION_ARGS_PASSTHRU, info),
		_type(type)
	{}

	inline Type type() const
	{
		return _type;
	}

	virtual AbstractError * clone() const
	{
		return new IOError(*this);
	}
protected:
	virtual std::wstring composeMessage() const
	{
		std::wstring result;
		switch (_type) {
			case TimeoutExpired:
				result = L"Timeout expired on I/O-device";
				break;
			case ConnectionAborted:
				result = L"Connection aborted on I/O-device";
				break;
			case DeviceIsNotOpen:
				result = L"I/O-device is not open";
				break;
			default:
				result = L"Unknown I/O-error";
		}
		appendInfo(result);
		return result;
	}
private:
	IOError();

	Type _type;
};

} // namespace isl

#endif


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

	static bool isInException(const Exception& e, Type type)
	{
		for (Exception::ErrorList::const_iterator i = e.errors().begin(); i != e.errors().end(); ++i) {
			IOError * err = dynamic_cast<IOError *>(*i);
			if (err && (err->_type == type)) {
				return true;
			}
		}
		return false;
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


#ifndef ISL__IO_ERROR__HXX
#define ISL__IO_ERROR__HXX

#include <isl/AbstractError.hxx>

namespace isl
{

class IOError : public AbstractError
{
public:
	enum Type {
		TimeoutExpired,
		ConnectionAborted,
		DeviceIsNotOpen
	};

	IOError(SOURCE_LOCATION_ARGS_DECLARATION, Type type, const std::string& info = std::string()) :
		AbstractError(SOURCE_LOCATION_ARGS_PASSTHRU),
		_type(type),
		_info(info)
	{}

	inline Type type() const
	{
		return _type;
	}
	inline const std::string& info() const
	{
		return _info;
	}

	virtual AbstractError * clone() const
	{
		return new IOError(*this);
	}
private:
	IOError();

	virtual std::string composeMessage() const
	{
		std::ostringstream oss;
		switch (_type) {
			case TimeoutExpired:
				oss << "Timeout expired on I/O-device";
				break;
			case ConnectionAborted:
				oss << "Connection aborted on I/O-device";
				break;
			case DeviceIsNotOpen:
				oss << "I/O-device is not open";
				break;
			default:
				oss << "Unknown I/O-error";
		}
		if (!_info.empty()) {
			oss << ": " << _info;
		}
		return oss.str();
	}

	Type _type;
	const std::string _info;
};

} // namespace isl

#endif


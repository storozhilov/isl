#ifndef ISL__SUBSYSTEM_ERROR__HXX
#define ISL__SUBSYSTEM_ERROR__HXX

#include <isl/AbstractError.hxx>

namespace isl
{

class SybsystemError : public AbstractInfoError
{
public:
	enum Type {
		CanNotChangeState
	};

	SybsystemError(SOURCE_LOCATION_ARGS_DECLARATION, Type type, const std::wstring& info = std::wstring()) :
		AbstractInfoError(SOURCE_LOCATION_ARGS_PASSTHRU, info),
		_type(type)
	{}

	inline Type type() const
	{
		return _type;
	}

	virtual AbstractError * clone() const
	{
		return new SybsystemError(*this);
	}
protected:
	virtual std::wstring composeMessage() const
	{
		std::wstring result;
		switch (_type) {
			case CanNotChangeState:
				result = L"Can not change subsystem state";
				break;
			default:
				result = L"Unknown subsystem error";
		}
		appendInfo(result);
		return result;
	}
private:
	SybsystemError();

	Type _type;
};

} // namespace isl

#endif


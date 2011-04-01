#ifndef ISL__NULLABLE_ENUM__HXX
#define ISL__NULLABLE_ENUM__HXX

#include <typeinfo>
#include <stdexcept>

namespace isl
{

/*------------------------------------------------------------------------------
 * NullableEnum
------------------------------------------------------------------------------*/

template <typename T> class NullableEnum
{
public:
	NullableEnum() :
		_value(0)
	{}
	NullableEnum(const T& value) :
		_value(value.clone())
	{}
	NullableEnum(const NullableEnum<T>& rhs) :
		_value(0)
	{
		if (!rhs.isNull()) {
			_value = rhs._value->clone();
		}
	}
	~NullableEnum()
	{
		reset();
	}

	NullableEnum<T>& operator=(const NullableEnum<T>& rhs)
	{
		T * oldValue = _value;
		if (rhs.isNull()) {
			_value = 0;
		} else {
			_value = rhs._value->clone();
		}
		if (oldValue) {
			delete oldValue;
		}
		return *this;
	}
	bool operator==(const NullableEnum<T>& rhs) const
	{
		if (isNull() && rhs.isNull()) {
			return true;
		} else if (!isNull() && ! rhs.isNull()) {
			return (typeid(*_value) == typeid(*(rhs._value)));
		} else {
			return false;
		}
	}
	bool operator!=(const NullableEnum<T>& rhs) const
	{
		return !operator==(rhs);
	}
	template <typename V> bool equals() const
	{
		if (isNull()) {
			return false;
		}
		return dynamic_cast<V *>(_value);
	}
	template <typename V> static NullableEnum<T> construct()
	{
		V v;
		return NullableEnum<T>(v);
	}
	const T& value() const
	{
		if (isNull()) {
			throw std::runtime_error("Enum value is NULL");				// TODO use specific exception
		}
		return *_value;
	}
	bool isNull() const
	{
		return !_value;
	}
	void reset()
	{
		if (_value) {
			delete _value;
		}
		_value = 0;
	}
private:
	T * _value;
};

} // namespace isl

#endif


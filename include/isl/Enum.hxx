#ifndef ISL__ENUM__HXX
#define ISL__ENUM__HXX

#include <typeinfo>

namespace isl
{

/*------------------------------------------------------------------------------
 * Extensible Enum class.
 * NOTE! Use Enum::construct<T>() to create a new value of the enum!
------------------------------------------------------------------------------*/

template <typename T> class Enum
{
public:
	Enum(const T& value) :
	//explicit Enum(const T& value) :
		_value(value.clone())
	{}
	Enum(const Enum<T>& rhs) :
	//explicit Enum(const Enum<T>& rhs) :
		_value(rhs._value->clone())
	{}
	~Enum()
	{
		delete _value;
	}

	Enum<T>& operator=(const Enum<T>& rhs)
	{
		T * oldValue = _value;
		_value = rhs._value->clone();
		delete oldValue;
		return *this;
	}
	bool operator==(const Enum<T>& rhs) const
	{
		return (typeid(*_value) == typeid(*(rhs._value)));
	}
	bool operator!=(const Enum<T>& rhs) const
	{
		return !operator==(rhs);
	}
	template <typename V> bool equals() const
	{
		return dynamic_cast<V *>(_value);
	}
	template <typename V> static Enum<T> construct()
	{
		V v;
		return Enum<T>(v);
	}
	const T& value() const
	{
		return *_value;
	}
private:
	Enum();

	T * _value;
};

} // namespace isl

#endif


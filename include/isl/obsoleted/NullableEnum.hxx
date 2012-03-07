#ifndef ISL__NULLABLE_ENUM__HXX
#define ISL__NULLABLE_ENUM__HXX

#include <typeinfo>
#include <stdexcept>

namespace isl
{

/*------------------------------------------------------------------------------
 * NullableEnum
------------------------------------------------------------------------------*/

//! "Extensible Nullable Enum" implementation.
/*!
    Enum values could be declared after enum declaration itself - only base class ('T') for enum values is needed.
    Example of use:

<pre>...
class AbstractEnumValue
{
public:
	virtual ~AbstractEnumValue() {}
	
	virtual AbstractEnumValue * clone() const = 0;
};

class FirstEnumValue: public AbstractEnumValue
{
public:
	virtual AbstractEnumValue * clone() const {
		return new FirstEnumValue(*this);
	}
};

class SecondEnumValue: public AbstractEnumValue
{
public:
	virtual AbstractEnumValue * clone() const {
		return new SecondEnumValue(*this);
	}
};
...
NullableEnum<AbstractEnumValue> v1(FirstEnumValue());
NullableEnum<AbstractEnumValue> v2(SecondEnumValue());
NullableEnum<AbstractEnumValue> v3;
if (v1 == v2) {
	std::wcout << L"v1 equals to v2" << std::endl;
} else {
	std::wcout << L"v1 NOT equals to v2" << std::endl;
}
if (v1.equals<SecondEnumValue>()) {
	std::wcout << L"v1 equals to SecondEnumValue" << std::endl;
} else {
	std::wcout << L"v1 NOT equals to SecondEnumValue" << std::endl;
}
if (v3.isNull()) {
	std::wcout << L"v3 contains null value" << std::endl;
} else {
	std::wcout << L"v3 contains NOT null value" << std::endl;
}
...</pre>
*/

template <typename T> class NullableEnum
{
public:
	//! Constructs enum holding NULL value
	NullableEnum() :
		_value(0)
	{}
	//! Constructs enum holding the passed value
	NullableEnum(const T& value) :
		_value(value.clone())
	{}
	//! Copying constructor
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

	//! Assignment opeartor
	inline NullableEnum<T>& operator=(const NullableEnum<T>& rhs)
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
	//! Comparing opeartor
	inline bool operator==(const NullableEnum<T>& rhs) const
	{
		if (isNull() && rhs.isNull()) {
			return true;
		} else if (!isNull() && ! rhs.isNull()) {
			return (typeid(*_value) == typeid(*(rhs._value)));
		} else {
			return false;
		}
	}
	//! Comparing opeartor
	inline bool operator!=(const NullableEnum<T>& rhs) const
	{
		return !operator==(rhs);
	}
	//! Type-based equation method
	template <typename V> inline bool equals() const
	{
		if (isNull()) {
			return false;
		}
		return dynamic_cast<V *>(_value);
	}
	//! Creating new enum type-based factory method which calls defauilt constructor of the value.
	template <typename V> inline static NullableEnum<T> construct()
	{
		V v;
		return NullableEnum<T>(v);
	}
	//! Returns reference to the value
	inline T& value() const
	{
		if (isNull()) {
			throw std::runtime_error("Enum value is NULL");				// TODO use specific exception
		}
		return *_value;
	}
	//! Returns const reference to the value
	inline const T& constValue() const
	{
		if (isNull()) {
			throw std::runtime_error("Enum value is NULL");				// TODO use specific exception
		}
		return *_value;
	}
	//! Inspects value to be NULL
	bool isNull() const
	{
		return !_value;
	}
	//! resets value to the NULL
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


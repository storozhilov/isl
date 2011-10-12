#ifndef ISL__ENUM__HXX
#define ISL__ENUM__HXX

#include <typeinfo>

namespace isl
{

/*------------------------------------------------------------------------------
 * Extensible Enum class.
 * NOTE! Use Enum<T>::construct<V>() to create a new value of the enum! (TODO ???)
------------------------------------------------------------------------------*/

//! "Extensible Enum" implementation.
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
Enum<AbstractEnumValue> v1(FirstEnumValue());
Enum<AbstractEnumValue> v2(SecondEnumValue());
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
...</pre>
*/

template <typename T> class Enum
{
public:
	//! Constructs enum from the value
	Enum(const T& value) :
		_value(value.clone())
	{}
	//! Copying constructor
	Enum(const Enum<T>& rhs) :
		_value(rhs._value->clone())
	{}
	~Enum()
	{
		delete _value;
	}

	//! Assignment opeartor
	inline Enum<T>& operator=(const Enum<T>& rhs)
	{
		T * oldValue = _value;
		_value = rhs._value->clone();
		delete oldValue;
		return *this;
	}
	//! Comparing opeartor
	inline bool operator==(const Enum<T>& rhs) const
	{
		// TODO Think about how to avoid RTTI usage?
		return (typeid(*_value) == typeid(*(rhs._value)));
	}
	//! Comparing opeartor
	inline bool operator!=(const Enum<T>& rhs) const
	{
		return !operator==(rhs);
	}
	//! Type-based equation method
	template <typename V> inline bool equals() const
	{
		return dynamic_cast<V *>(_value);
	}
	//! Creating new enum type-based factory method which calls defauilt constructor of the value.
	template <typename V> inline static Enum<T> construct()
	{
		V v;
		return Enum<T>(v);
	}
	//! Returns reference to the value
	inline T& value() const
	{
		return *_value;
	}
	//! Returns const reference to the value
	inline const T& constValue() const
	{
		return *_value;
	}
private:
	Enum();

	T * _value;
};

} // namespace isl

#endif


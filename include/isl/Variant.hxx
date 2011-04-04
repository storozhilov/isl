#ifndef ISL__VARIANT__HXX
#define ISL__VARIANT__HXX

#include <isl/Utf8TextCodec.hxx>
#include <stdexcept>
#include <iostream>
#include <string>
#include <sstream>
#include <memory>

namespace isl
{

class AbstractVariantFormatter;
class NullVariantFormatter;

/*------------------------------------------------------------------------------
 * VariantOperator
 * ---------------------------------------------------------------------------*/

template <typename T> class VariantOperator
{
public:
	static std::wstring serialize(const T& value)
	{
		throw std::runtime_error("Serializing of this variant type is not implemented");
	}
	static T deserialize(const std::wstring& serializedValue)
	{
		throw std::runtime_error("Deserializing of this variant type is not implemented");
	}
	static int typeId()
	{
		throw std::runtime_error("Type ID is not implemented");
	}
	static AbstractVariantFormatter * createFormatter();
};

/*------------------------------------------------------------------------------
 * Variant
 * ---------------------------------------------------------------------------*/

class Variant
{
public:
	enum Type {
		NullType = 0x00,
		IntegerType = 0x01,
		StringType = 0x02,
		WStringType = 0x03,
		// TODO Other types
		UserType = 0x80
	};

	Variant();
	template <typename T> Variant(const T& value);

	bool isNull() const
	{
		return _typeId == NullType;
	}
	template <typename T> T value() const
	{
		if ((_typeId != VariantOperator<T>::typeId()) || (_typeId == NullType)) {
			return T();
		}
		return VariantOperator<T>::deserialize(_serializedValue);
	}
	template <typename T> void setValue(const T& newValue)
	{
		_formatter.reset(VariantOperator<T>::createFormatter());
		_typeId = VariantOperator<T>::typeId();
		_serializedValue = VariantOperator<T>::serialize(newValue);
	}
	void resetValue();
	inline std::wstring serializedValue() const
	{
		return _serializedValue;
	}
	inline std::wstring format(const std::wstring& fmt) const;
private:
	Variant(const Variant&);

	Variant& operator=(const Variant&);
	
	int _typeId;
	std::auto_ptr<AbstractVariantFormatter> _formatter;
	std::wstring _serializedValue;
};

/*------------------------------------------------------------------------------
 * AbstractVariantFormatter
 * ---------------------------------------------------------------------------*/

// TODO Template for char and wchar_t?
class AbstractVariantFormatter
{
public:
	AbstractVariantFormatter()
	{}
	virtual ~AbstractVariantFormatter()
	{}

	virtual std::wstring format(const Variant& var, const std::wstring& fmt) = 0;
};

class NullVariantFormatter : public AbstractVariantFormatter
{
public:
	NullVariantFormatter() :
		AbstractVariantFormatter()
	{}

	virtual std::wstring format(const Variant& var, const std::wstring& fmt)
	{
		return L"[NULL]";
	}
};

/*------------------------------------------------------------------------------
 * VariantOperator definitions
 * ---------------------------------------------------------------------------*/

template <typename T> AbstractVariantFormatter * VariantOperator<T>::createFormatter()
{
	return new NullVariantFormatter();
}

/*------------------------------------------------------------------------------
 * Variant definitions
 * ---------------------------------------------------------------------------*/

Variant::Variant() :
	_typeId(NullType),
	_formatter(new NullVariantFormatter()),
	_serializedValue()
{}

template <typename T> Variant::Variant(const T& value) :
	_typeId(NullType),
	_formatter(new NullVariantFormatter()),
	_serializedValue()
{
	setValue<T>(value);
}

void Variant::resetValue()
{
	_formatter.reset(new NullVariantFormatter());
	_typeId = NullType;
	_serializedValue.clear();
}

std::wstring Variant::format(const std::wstring& fmt) const
{
	return (_formatter.get()) ? _formatter->format(*this, fmt) : std::wstring(L"[EMPTY FORMATTER!]");
}

/*------------------------------------------------------------------------------
 * AbstractVariantFormatter descendants definitions and
 * VariantOperator template specifications for basic types
 * TODO Other types
 * ---------------------------------------------------------------------------*/

// int

class IntegerVariantFormatter : public AbstractVariantFormatter
{
public:
	IntegerVariantFormatter() :
		AbstractVariantFormatter()
	{}

	virtual std::wstring format(const Variant& var, const std::wstring& fmt)
	{
		// TODO
		return var.serializedValue();
	}
};

template <> class VariantOperator <int>
{
public:
	static std::wstring serialize(const int& value)
	{
		std::wostringstream oss;
		oss << value;
		return oss.str();
	}
	static int deserialize(const std::wstring serializedValue)
	{
		int result;
		std::wistringstream iss(serializedValue);
		iss >> result;
		return result;
	}
	static int typeId()
	{
		return Variant::IntegerType;
	}
	static AbstractVariantFormatter * createFormatter()
	{
		return new IntegerVariantFormatter();
	}
};

// std::string

class StringVariantFormatter : public AbstractVariantFormatter
{
public:
	StringVariantFormatter() :
		AbstractVariantFormatter()
	{}

	virtual std::wstring format(const Variant& var, const std::wstring& fmt)
	{
		return var.serializedValue();
	}
};

template <> class VariantOperator <std::string>
{
public:
	static std::wstring serialize(const std::string& value)
	{
		//return value;
		return Utf8TextCodec().decode(value);
	}
	static std::string deserialize(const std::wstring serializedValue)
	{
		return Utf8TextCodec().encode(serializedValue);
	}
	static int typeId()
	{
		return Variant::StringType;
	}
	static AbstractVariantFormatter * createFormatter()
	{
		return new StringVariantFormatter();
	}
};

// std::wstring

class WStringVariantFormatter : public AbstractVariantFormatter
{
public:
	WStringVariantFormatter() :
		AbstractVariantFormatter()
	{}

	virtual std::wstring format(const Variant& var, const std::wstring& fmt)
	{
		return var.serializedValue();
	}
};

template <> class VariantOperator <std::wstring>
{
public:
	static std::wstring serialize(const std::wstring& value)
	{
		return value;
	}
	static std::wstring deserialize(const std::wstring serializedValue)
	{
		return serializedValue;
	}
	static int typeId()
	{
		return Variant::WStringType;
	}
	static AbstractVariantFormatter * createFormatter()
	{
		return new WStringVariantFormatter();
	}
};

} // namespace isl

#endif


#ifndef ISL__VARIANT__HXX
#define ISL__VARIANT__HXX

#include <isl/String.hxx>
#include <isl/Date.hxx>
#include <isl/Time.hxx>
#include <isl/DateTime.hxx>
#include <stdexcept>
#include <iostream>
#include <string>
#include <sstream>
#include <memory>

namespace isl
{

class AbstractVariantFormatter;
class NullVariantFormatter;

//! Variant subsystem helper class with static members only.
/*!
    You should specialize this class to "variantize" you datatype.
*/
template <typename T> class VariantOperator
{
public:
	//! Serializes value
	static std::wstring serialize(const T& value)
	{
		throw std::runtime_error("Serializing of this variant type is not implemented");
	}
	//! Deserializes value
	static T deserialize(const std::wstring& serializedValue)
	{
		throw std::runtime_error("Deserializing of this variant type is not implemented");
	}
	//! Returns particular type ID
	static int typeId()
	{
		throw std::runtime_error("Type ID is not implemented for the requested variant type");
	}
	//! Creating formatter for particular type factory method
	static AbstractVariantFormatter * createFormatter();
};

//! C++ extensible Variant implementation with user types support
/*!
    In order to "variantize" variables of your type you should:

    - Implement an isl::AbstractVariantFormatter's descensdant which should know how to format variables of your type;
    - Reserve an integer value as an ID for your type (should be not less than isl::Variant::UserType);
    - Specialize isl::VariantOperator<T> class definition for your type, where you should define:
      - How to serialize values of your type by redefinintion of std::wstring isl::VariantOperator<T>::serialize(const T& value);
      - How to deserialize values of your type by redefinintion of T isl::VariantOperator<T>::deserialize(const std::wstring& serializedValue);
      - What ID to return for your type by redefinintion of int isl::VariantOperator<T>::typeId();
      - What formatter to create for your type by redefinition of AbstractVariantFormatter * isl::VariantOperator<T>::createFormatter().

    Note: String variant values should be constructed in the followed manner (call ctor explicitly):

<pre>...
isl::Variant v1(std::string("Variant with one byte character string"));
isl::Variant v2(std::wstring(L"Variant with wide character string"));
...</pre>
*/
class Variant
{
public:
	//! IDs for supported variant value types
	enum TypeId {
		//! Special type for NULL values
		NullType = 0x00,
		//! Character variant type ID
		CharType = 0x01,
		//! Wide character variant type ID
		WCharType = 0x02,
		//! Integer variant type ID
		IntegerType = 0x03,
		//! Double variant type ID
		DoubleType = 0x04,
		//! String variant type ID
		StringType = 0x05,
		//! Wide String variant type ID
		WStringType = 0x06,
		//! Date type ID
		DateType = 0x07,
		//! Time type ID
		TimeType = 0x08,
		//! DateTime type ID
		DateTimeType = 0x09,
		// TODO Other types
		//! Lower boundary for user variant type IDs
		UserType = 0x80
	};

	Variant();
	//! Type-based copying constructor
	template <typename T> Variant(const T& value);
	//! Copying constructor
	Variant(const Variant& rhs);

	//! Assignment operator
	Variant& operator=(const Variant& rhs);

	//! Returns true if the Variant holds NULL value
	bool isNull() const
	{
		return _typeId == NullType;
	}
	//! Fetches value of the requested type
	template <typename T> T value() const
	{
		if ((_typeId != VariantOperator<T>::typeId()) || (_typeId == NullType)) {
			return T();
		}
		return VariantOperator<T>::deserialize(_serializedValue);
	}
	//! Sets value
	template <typename T> void setValue(const T& newValue)
	{
		_formatter.reset(VariantOperator<T>::createFormatter());
		_typeId = VariantOperator<T>::typeId();
		_serializedValue = VariantOperator<T>::serialize(newValue);
	}
	//! Type-based assignment operator
	template <typename T> Variant& operator=(const T& newValue)
	{
		setValue<T>(newValue);
		return *this;
	}
	//! Resets Variant to be holding NULL value
	void resetValue();
	//! Returns serialized repesentation of the value
	inline std::wstring serializedValue() const
	{
		return _serializedValue;
	}
	//! Formats Variant's value to one byte character string
	inline std::string format(const std::string& fmt) const;
	//! Formats Variant's value to wide character string
	inline std::wstring format(const std::wstring& fmt) const;
private:
	int _typeId;
	std::auto_ptr<AbstractVariantFormatter> _formatter;
	std::wstring _serializedValue;
};

//! Variant subsystem helper class
/*!
    Implement this class ancestor to "variantize" your datatype
*/
class AbstractVariantFormatter
{
public:
	AbstractVariantFormatter()
	{}
	virtual ~AbstractVariantFormatter()
	{}

	//! Cloning object factory method
	virtual AbstractVariantFormatter * clone() const = 0;
	//! Format composition method
	virtual std::wstring compose(const Variant& var, const std::wstring& fmt) const = 0;
};

//! NULL-variant formatter
class NullVariantFormatter : public AbstractVariantFormatter
{
public:
	NullVariantFormatter() :
		AbstractVariantFormatter()
	{}

	virtual AbstractVariantFormatter * clone() const
	{
		return new NullVariantFormatter(*this);
	}
	virtual std::wstring compose(const Variant& var, const std::wstring& fmt) const
	{
		return L"[null]";
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

Variant::Variant(const Variant& rhs) :
	_typeId(rhs._typeId),
	_formatter(rhs._formatter->clone()),
	_serializedValue(rhs._serializedValue)
{}

Variant& Variant::operator=(const Variant& rhs)
{
	_typeId = rhs._typeId;
	_formatter.reset(rhs._formatter->clone());
	_serializedValue = rhs._serializedValue;
	return *this;
}

void Variant::resetValue()
{
	_formatter.reset(new NullVariantFormatter());
	_typeId = NullType;
	_serializedValue.clear();
}

std::string Variant::format(const std::string& fmt) const
{
	return (_formatter.get()) ? String::utf8Encode(_formatter->compose(*this, String::utf8Decode(fmt))) : std::string("[ERROR: Empty formatter!]");
}

std::wstring Variant::format(const std::wstring& fmt) const
{
	return (_formatter.get()) ? _formatter->compose(*this, fmt) : std::wstring(L"[ERROR: Empty formatter!]");
}

/*------------------------------------------------------------------------------
 * AbstractVariantFormatter descendants definitions and
 * VariantOperator template specifications for basic types
 * TODO Other types
 * ---------------------------------------------------------------------------*/

// char

template <> class VariantOperator <char>
{
private:
	class CharFormatter : public AbstractVariantFormatter
	{
	public:
		CharFormatter() :
			AbstractVariantFormatter()
		{}

		virtual AbstractVariantFormatter * clone() const
		{
			return new CharFormatter(*this);
		}
		virtual std::wstring compose(const Variant& var, const std::wstring& fmt) const
		{
			// TODO Handle fmt
			return var.serializedValue();
		}
	};
public:
	static std::wstring serialize(const char& value)
	{
		return std::wstring(1, value);
	}
	static char deserialize(const std::wstring& serializedValue)
	{
		return (serializedValue.length() > 0) ? serializedValue[0] : '\0';
	}
	static int typeId()
	{
		return Variant::CharType;
	}
	static AbstractVariantFormatter * createFormatter()
	{
		return new CharFormatter();
	}
};

// wchar_t

template <> class VariantOperator <wchar_t>
{
private:
	class WCharFormatter : public AbstractVariantFormatter
	{
	public:
		WCharFormatter() :
			AbstractVariantFormatter()
		{}

		virtual AbstractVariantFormatter * clone() const
		{
			return new WCharFormatter(*this);
		}
		virtual std::wstring compose(const Variant& var, const std::wstring& fmt) const
		{
			// TODO Handle fmt
			return var.serializedValue();
		}
	};
public:
	static std::wstring serialize(const wchar_t& value)
	{
		return std::wstring(1, value);
	}
	static wchar_t deserialize(const std::wstring& serializedValue)
	{
		return (serializedValue.length() > 0) ? serializedValue[0] : L'\0';
	}
	static int typeId()
	{
		return Variant::WCharType;
	}
	static AbstractVariantFormatter * createFormatter()
	{
		return new WCharFormatter();
	}
};

// int

template <> class VariantOperator <int>
{
private:
	class IntegerFormatter : public AbstractVariantFormatter
	{
	public:
		IntegerFormatter() :
			AbstractVariantFormatter()
		{}

		virtual AbstractVariantFormatter * clone() const
		{
			return new IntegerFormatter(*this);
		}
		virtual std::wstring compose(const Variant& var, const std::wstring& fmt) const
		{
			// TODO Handle fmt
			return var.serializedValue();
		}
	};
public:
	static std::wstring serialize(const int& value)
	{
		std::wostringstream oss;
		oss << value;
		return oss.str();
	}
	static int deserialize(const std::wstring& serializedValue)
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
		return new IntegerFormatter();
	}
};

// double

template <> class VariantOperator <double>
{
private:
	class DoubleFormatter : public AbstractVariantFormatter
	{
	public:
		DoubleFormatter() :
			AbstractVariantFormatter()
		{}

		virtual AbstractVariantFormatter * clone() const
		{
			return new DoubleFormatter(*this);
		}
		virtual std::wstring compose(const Variant& var, const std::wstring& fmt) const
		{
			// TODO Handle fmt
			return var.serializedValue();
		}
	};
public:
	static std::wstring serialize(const double& value)
	{
		std::wostringstream oss;
		oss << value;
		return oss.str();
	}
	static double deserialize(const std::wstring& serializedValue)
	{
		double result;
		std::wistringstream iss(serializedValue);	// TODO Fractional part cutted - should be fixed
		iss >> result;
		return result;
	}
	static double typeId()
	{
		return Variant::DoubleType;
	}
	static AbstractVariantFormatter * createFormatter()
	{
		return new DoubleFormatter();
	}
};

// std::string

template <> class VariantOperator <std::string>
{
private:
	class StringFormatter : public AbstractVariantFormatter
	{
	public:
		StringFormatter() :
			AbstractVariantFormatter()
		{}

		virtual AbstractVariantFormatter * clone() const
		{
			return new StringFormatter(*this);
		}
		virtual std::wstring compose(const Variant& var, const std::wstring& fmt) const
		{
			// TODO Handle fmt
			return var.serializedValue();
		}
	};
public:
	static std::wstring serialize(const std::string& value)
	{
		return String::utf8Decode(value);
	}
	static std::string deserialize(const std::wstring& serializedValue)
	{
		return String::utf8Encode(serializedValue);
	}
	static int typeId()
	{
		return Variant::StringType;
	}
	static AbstractVariantFormatter * createFormatter()
	{
		return new StringFormatter();
	}
};

// std::wstring

template <> class VariantOperator <std::wstring>
{
private:
	class WStringFormatter : public AbstractVariantFormatter
	{
	public:
		WStringFormatter() :
			AbstractVariantFormatter()
		{}

		virtual AbstractVariantFormatter * clone() const
		{
			return new WStringFormatter(*this);
		}
		virtual std::wstring compose(const Variant& var, const std::wstring& fmt) const
		{
			// TODO Handle fmt
			return var.serializedValue();
		}
	};
public:
	static std::wstring serialize(const std::wstring& value)
	{
		return value;
	}
	static std::wstring deserialize(const std::wstring& serializedValue)
	{
		return serializedValue;
	}
	static int typeId()
	{
		return Variant::WStringType;
	}
	static AbstractVariantFormatter * createFormatter()
	{
		return new WStringFormatter();
	}
};

// Date

template <> class VariantOperator <Date>
{
private:
	class DateFormatter : public AbstractVariantFormatter
	{
	public:
		DateFormatter() :
			AbstractVariantFormatter()
		{}

		virtual AbstractVariantFormatter * clone() const
		{
			return new DateFormatter(*this);
		}
		virtual std::wstring compose(const Variant& var, const std::wstring& fmt) const
		{
			return var.value<Date>().toWString(fmt.empty() ? Date::IsoOutputWFormat : fmt);
		}
	};
public:
	static std::wstring serialize(const Date& value)
	{
		return value.toWString(Date::IsoOutputWFormat);
	}
	static Date deserialize(const std::wstring& serializedValue)
	{
		return Date::fromWString(serializedValue, Date::IsoInputWFormat);
	}
	static int typeId()
	{
		return Variant::DateType;
	}
	static AbstractVariantFormatter * createFormatter()
	{
		return new DateFormatter();
	}
};

// Time

template <> class VariantOperator <Time>
{
private:
	class TimeFormatter : public AbstractVariantFormatter
	{
	public:
		TimeFormatter() :
			AbstractVariantFormatter()
		{}

		virtual AbstractVariantFormatter * clone() const
		{
			return new TimeFormatter(*this);
		}
		virtual std::wstring compose(const Variant& var, const std::wstring& fmt) const
		{
			return var.value<Time>().toWString(fmt.empty() ? Time::IsoOutputWFormat : fmt);
		}
	};
public:
	static std::wstring serialize(const Time& value)
	{
		return value.toWString(Time::IsoOutputWFormat);
	}
	static Time deserialize(const std::wstring& serializedValue)
	{
		return Time::fromWString(serializedValue, Time::IsoInputWFormat);
	}
	static int typeId()
	{
		return Variant::TimeType;
	}
	static AbstractVariantFormatter * createFormatter()
	{
		return new TimeFormatter();
	}
};

// DateTime

template <> class VariantOperator <DateTime>
{
private:
	class DateTimeFormatter : public AbstractVariantFormatter
	{
	public:
		DateTimeFormatter() :
			AbstractVariantFormatter()
		{}

		virtual AbstractVariantFormatter * clone() const
		{
			return new DateTimeFormatter(*this);
		}
		virtual std::wstring compose(const Variant& var, const std::wstring& fmt) const
		{
			return var.value<DateTime>().toWString(fmt.empty() ? DateTime::IsoOutputWFormat : fmt);
		}
	};
public:
	static std::wstring serialize(const DateTime& value)
	{
		return value.toWString(DateTime::IsoOutputWFormat);
	}
	static DateTime deserialize(const std::wstring& serializedValue)
	{
		return DateTime::fromWString(serializedValue, DateTime::IsoInputWFormat);
	}
	static int typeId()
	{
		return Variant::DateTimeType;
	}
	static AbstractVariantFormatter * createFormatter()
	{
		return new DateTimeFormatter();
	}
};

} // namespace isl

#endif

#ifndef ISL__ABSTRACT_ERROR__HXX
#define ISL__ABSTRACT_ERROR__HXX

#include <string>
#include <memory>

// Yes, macro are used here... Sad, but true...
// TODO Move this staff to more appropriate place
#ifdef __GNUC__
#define SOURCE_LOCATION_ARGS __FILE__, __LINE__, __PRETTY_FUNCTION__
#else
#define SOURCE_LOCATION_ARGS __FILE__, __LINE__, __func__
#endif
#define SOURCE_LOCATION_ARGS_FILE __file
#define SOURCE_LOCATION_ARGS_LINE __line
#define SOURCE_LOCATION_ARGS_FUNCTION __function
#define SOURCE_LOCATION_ARGS_DECLARATION const char * SOURCE_LOCATION_ARGS_FILE, unsigned int SOURCE_LOCATION_ARGS_LINE, const char * SOURCE_LOCATION_ARGS_FUNCTION
#define SOURCE_LOCATION_ARGS_PASSTHRU SOURCE_LOCATION_ARGS_FILE, SOURCE_LOCATION_ARGS_LINE, SOURCE_LOCATION_ARGS_FUNCTION

namespace isl
{

class AbstractError
{
public:
	class AbstractType						// Additional abstraction level for handling several error
	{								// types in one error class and to customize error message
	public:								// composing
		virtual ~AbstractType() {}
		
		virtual AbstractType * clone() const = 0;
		virtual std::wstring message() const = 0;
	};
	class DefaultType : public AbstractType				// Simple error type with the message only
	{
	public:
		DefaultType(const std::wstring& message) :
			AbstractType(),
			_message(message)
		{}

		virtual AbstractType * clone() const { return new DefaultType(*this); }
		virtual std::wstring message() const { return _message; }
	private:
		DefaultType();

		std::wstring _message;
	};

	AbstractError(const AbstractType& type, SOURCE_LOCATION_ARGS_DECLARATION);
	AbstractError(const std::wstring& message, SOURCE_LOCATION_ARGS_DECLARATION);
	AbstractError(const AbstractError& rhs);
	virtual ~AbstractError();
	
	AbstractError& operator=(const AbstractError& rhs);

	const AbstractType& type() const;
	template <typename T> bool typeOf()
	{
		return (dynamic_cast<T *>(_typePtr.get()));
	}
	const wchar_t * message() const throw();
	const wchar_t * debug() const throw();
	const char * file() const throw();
	unsigned int line() const throw();
	const char * function() const throw();
	template <typename T> bool instanceOf()
	{
		return (dynamic_cast<T *>(this));
	}

	virtual AbstractError * clone() const = 0;
private:
	AbstractError();

	void initDebug();

	std::auto_ptr<AbstractType> _typePtr;
	std::wstring _message;
	std::wstring _debug;
	std::string _file;
	unsigned int _line;
	std::string _function;
};

} // namespace isl

#endif


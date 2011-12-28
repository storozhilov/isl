#ifndef ISL__EXCEPTION__HXX
#define ISL__EXCEPTION__HXX

#include <isl/AbstractError.hxx>
#include <exception>
#include <list>

namespace isl
{

class Exception : public std::exception
{
public:
	typedef std::list<AbstractError *> ErrorList;

	Exception(const AbstractError& error);
	Exception(const Exception& exception);
	~Exception() throw();

	// TODO To override operator=()
	void addError(const AbstractError& error);
	template <typename T> bool hasError() const
	{
		for (ErrorList::const_iterator i = _errors.begin(); i != _errors.end(); ++i) {
			if ((*i)->instanceOf<T>()) {
				return true;
			}
		}
		return false;
	}
	template <typename T> bool hasOneError() const
	{
		return (_errors.size() == 1) && (*_errors.begin())->instanceOf<T>();
	}
	const wchar_t * message() const throw();
	const wchar_t * debug() const throw();

	virtual const char * what() const throw();
private:
	Exception();

	ErrorList _errors;
	std::wstring _message;
	std::wstring _debug;
	std::string _what;
};

} // namespace isl

#endif

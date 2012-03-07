#ifndef ISL__REFERENCE__HXX
#define ISL__REFERENCE__HXX

#include <isl/Exception.hxx>
#include <isl/AbstractError.hxx>
#include <memory>

namespace isl
{

class AbstractReferenceError : public AbstractError
{
public:
	AbstractReferenceError(const std::wstring& message, SOURCE_LOCATION_ARGS_DECLARATION) :
		AbstractError(message, SOURCE_LOCATION_ARGS_PASSTHRU)
	{}
private:
	AbstractReferenceError();
};

class NoReferenceValueError : public AbstractReferenceError
{
public:
	NoReferenceValueError(SOURCE_LOCATION_ARGS_DECLARATION) :
		AbstractReferenceError(L"Instance has no value", SOURCE_LOCATION_ARGS_PASSTHRU)
	{}
	
	virtual AbstractError * clone() const
	{
		return new NoReferenceValueError(*this);
	}
private:
	NoReferenceValueError();
};

class NoReferenceInstanceError : public AbstractReferenceError
{
public:
	NoReferenceInstanceError(SOURCE_LOCATION_ARGS_DECLARATION) :
		AbstractReferenceError(L"Reference has no instance", SOURCE_LOCATION_ARGS_PASSTHRU)
	{}
	
	virtual AbstractError * clone() const
	{
		return new NoReferenceInstanceError(*this);
	}
private:
	NoReferenceInstanceError();
};

template <typename T> class Reference
{
private:
	class Instance
	{
	public:
		Instance(T * valuePtr) :
			_refCount(0),
			_valuePtr(valuePtr)
		{
			if (!_valuePtr.get()) {
				throw Exception(NoReferenceValueError(SOURCE_LOCATION_ARGS));
			}
		}
	
		int incRefCount()
		{
			return ++_refCount;
		}
		int decRefCount()
		{
			return --_refCount;
		}
		int refCount() const
		{
			return _refCount;
		}
		T * valuePtr() const
		{
			return _valuePtr.get();
		}
	private:
		Instance();
		Instance(const Instance& rhs);

		Instance& operator=(const Instance& rhs);

		int _refCount;
		std::auto_ptr<T> _valuePtr;
	};
public:
	Reference() :
		_instance(0)
	{}
	Reference(const T& value) :								// Using a copy constructor to create an instance value
		_instance(new Instance(new T(value)))
	{
		_instance->incRefCount();
	}
	Reference(T * valuePtr) :								// Taking an ownership to the value pointed
		_instance(new Instance(valuePtr))
	{
		_instance->incRefCount();
	}
	Reference(const Reference<T>& rhs) :
		_instance(rhs._instance)
	{
		if (_instance) {
			_instance->incRefCount();
		}
	}

	~Reference()
	{
		resetInstance();
	}

	Reference<T>& operator=(const Reference<T>& rhs)
	{
		resetInstance();
		_instance = rhs._instance;
		if (_instance) {
			_instance->incRefCount();
		}
		return *this;
	}

	bool isNull() const
	{
		return !_instance;
	}
	void setValue(const T& value)								// Using a copy constructor to create an instance value
	{
		resetInstance();
		_instance = new Instance(new T(value));
		_instance->incRefCount();
	}
	void acquire(T * valuePtr)								// Taking an ownership of the value pointed
	{
		resetInstance();
		_instance = new Instance(valuePtr);
		_instance->incRefCount();
	}
	void resetInstance()
	{
		if (_instance) {
			if (_instance->decRefCount() <= 0) {
				delete _instance;
			}
			_instance = 0;
		}
	}
	T * pointer() const
	{
		if (!_instance) {
			throw Exception(NoReferenceInstanceError(SOURCE_LOCATION_ARGS));
		}
		return _instance->valuePtr();
	}
	inline T * ptr() const
	{
		return pointer();
	}
	inline T& value() const
	{
		return *(operator->());
	}
	inline T& val() const
	{
		return value();
	}
	inline T& operator*() const
	{
		return *(operator->());
	}
	T * operator->() const
	{
		T * ptr = pointer();
		if (!ptr) {
			throw Exception(NoReferenceValueError(SOURCE_LOCATION_ARGS));
		}
		return ptr;
	}
	bool operator==(const Reference<T>& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return _instance == rhs._instance;
	}
	bool operator!=(const Reference<T>& rhs) const
	{
		if (isNull() || rhs.isNull()) {
			return false;
		}
		return _instance != rhs._instance;
	}
private:
	Instance * _instance;
};

} // namespace isl

#endif


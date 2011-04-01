#ifndef ISL__SHARED_OBJECT__HXX
#define ISL__SHARED_OBJECT__HXX

namespace isl
{

template <typename T> class Instance;

template <typename T> class Link
{
public:
	Link() :
		_instance(0)
	{}
	Link(const T& val) :
		_instance(Instance<T>::createInstance(val))
	{
		if (_instance) {
			_instance->ref();
		}
	}
	Link(const Link<T>& rhs) :
		_instance(rhs._instance)
	{
		if (_instance) {
			_instance->ref();
		}
	}
	~Link()
	{
		resetInstance();
	}

	Link<T>& operator=(const Link<T>& rhs)
	{
		resetInstance();
		_instance = rhs._instance;
		if (_instance) {
			_instance->ref();
		}
		return *this;
	}

	bool isNull() const
	{
		return !_instance;
	}
	void setInstance(const T& val)
	{
		resetInstance();
		_instance = Instance<T>::createInstance(val);
		if (_instance) {
			_instance->ref();
		}
	}
	void resetInstance()
	{
		if (_instance) {
			if (_instance->deref() <= 0) {
				delete _instance;
			}
			_instance = 0;
		}
	}
	T& val()
	{
		if (!_instance) {
			throw std::runtime_error("Link is null");
		}
		return _instance->_val;
	}
private:
	Instance<T> * _instance;
};

template <typename T> class Instance
{
private:
	Instance() :
		_refCount(0),
		_val()
	{}
	Instance(const T& val) :
		_refCount(0),
		_val(val)
	{}
	Instance(const Instance& rhs);

	Instance& operator=(const Instance& rhs);

	int ref()
	{
		return ++_refCount;
	}
	int deref()
	{
		return --_refCount;
	}
	int refCount() const
	{
		return _refCount;
	}

	static Instance<T> * createInstance(const T& val)
	{
		return new Instance<T>(val);
	}

	int _refCount;
	T _val;

	friend class Link<T>;
};

} // namespace isl

#endif


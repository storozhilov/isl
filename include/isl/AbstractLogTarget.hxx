#ifndef ISL__ABSTRACT_LOG_TARGET__HXX
#define ISL__ABSTRACT_LOG_TARGET__HXX

namespace isl
{

class AbstractLogDevice;

class AbstractLogTarget
{
public:
	AbstractLogTarget();
	virtual ~AbstractLogTarget();
protected:
	virtual AbstractLogDevice *createDevice() const = 0;

	friend class LogDispatcher;
};

} // namespace isl

#endif


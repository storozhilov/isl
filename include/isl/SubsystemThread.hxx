#ifndef ISL__SUBSYSTEM_THREAD__HXX
#define ISL__SUBSYSTEM_THREAD__HXX

#include <isl/Thread.hxx>
#include <string>

namespace isl
{

class AbstractSubsystem;

class SubsystemThread : public Thread
{
public:
	SubsystemThread(AbstractSubsystem& subsystem, bool awaitStartup = false);
	virtual ~SubsystemThread();

	inline AbstractSubsystem& subsystem() const
	{
		return _subsystem;
	}
protected:
	bool shouldTerminate();
	bool awaitTermination(Timeout timeout = Timeout::defaultTimeout());
private:
	AbstractSubsystem& _subsystem;
};

} // namespace isl

#endif

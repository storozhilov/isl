#ifndef ISL__SUBSYSTEM_THREAD__HXX
#define ISL__SUBSYSTEM_THREAD__HXX

#include <isl/Thread.hxx>
#include <string>

namespace isl
{

class AbstractSubsystem;

//! Subsystem-aware thread with is controlled by it's subsystem
class SubsystemThread : public Thread
{
public:
	//! Constructs subsystem-aware thread
	/*!
	  \param subsystem Reference to subsystem object new thread is controlled by
	  \param awaitStartup Await for thread to be started
	*/
	SubsystemThread(AbstractSubsystem& subsystem, bool awaitStartup = false);
	virtual ~SubsystemThread();
	//! Returns reference to subsystem object
	inline AbstractSubsystem& subsystem() const
	{
		return _subsystem;
	}
protected:
	//! Returns TRUE if the thread should be terminated due to it's subsystem state
	bool shouldTerminate();
	//! Awaiting for thread termination method
	/*!
	  \param timeout Timeout to wait
	  \return TRUE if the thread has been terminated
	*/
	bool awaitTermination(Timeout timeout = Timeout::defaultTimeout());
private:
	AbstractSubsystem& _subsystem;
};

} // namespace isl

#endif

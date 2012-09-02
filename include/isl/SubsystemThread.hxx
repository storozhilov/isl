#ifndef ISL__SUBSYSTEM_THREAD__HXX
#define ISL__SUBSYSTEM_THREAD__HXX

#include <isl/Thread.hxx>
#include <isl/WaitCondition.hxx>
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
	  \param autoStop Thread will be terminated on subsytem's stop opeartion if TRUE
	  \param awaitStartup Await for thread to be started
	*/
	SubsystemThread(AbstractSubsystem& subsystem, bool autoStop = true, bool awaitStartup = false);
	virtual ~SubsystemThread();
	//! Returns reference to subsystem object
	inline AbstractSubsystem& subsystem() const
	{
		return _subsystem;
	}
	//! Returns TRUE if the thread should be terminated due to it's subsystem state
	bool shouldTerminate();
	//! Awaiting for thread termination method
	/*!
	  \param timeout Timeout to wait
	  \return TRUE if the thread has been terminated
	*/
	bool awaitShouldTerminate(Timeout timeout = Timeout::defaultTimeout());
	//! Sets should terminate flag to the new value
	/*!
	  \param newValue New value for the should terminate flag
	*/
	void setShouldTerminate(bool newValue);
private:
	AbstractSubsystem& _subsystem;
	bool _autoStop;
	bool _shouldTerminate;
	WaitCondition _shouldTerminateCond;

	friend class AbstractSubsystem;
};

} // namespace isl

#endif

#ifndef ISL__SUBSYSTEM__HXX
#define ISL__SUBSYSTEM__HXX

#include <isl/ReadWriteLock.hxx>
#include <isl/Mutex.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Debug.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/AbstractThread.hxx>
#include <string>
#include <algorithm>

namespace isl
{

//! Server subsystem base class
/*!
  Basic component of any server acconrding to <a href="http://en.wikipedia.org/wiki/Composite_pattern">Composite Design Pattern</a>.
*/
class Subsystem
{
public:
	//! Constructs a new subsystem
	/*!
	  \param owner The owner subsystem of the new subsystem
	*/
	Subsystem(Subsystem * owner);
	//! Destructor
	virtual ~Subsystem();
	//! Returns an owner of the subsystem
	inline Subsystem * owner() const
	{
		return _owner;
	}
	//! Asynchronously starts subsystem
	void start();
	//! Synchronously stops subsystem
	void stop();
protected:
	//! Subsystem-aware abstract thread
	class AbstractThread : public ::isl::AbstractThread
	{
	public:
		//! Constructs subsystem-aware thread
		/*!
		  \param subsystem Reference to subsystem object new thread is controlled by
		  \param autoStart TRUE if thread should be automatically started on subsystem's start operation
		  \param autoStop Thread will be terminated and joined to on subsytem's stop operation if TRUE or you should do it yourself in some other thread otherwise
		  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
		  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
		*/
		AbstractThread(Subsystem& subsystem, bool autoStart = true, bool autoStop = true, bool isTrackable = false, bool awaitStartup = false);
		//! Destructor
		virtual ~AbstractThread();
		//! Returns reference to subsystem object
		inline Subsystem& subsystem() const
		{
			return _subsystem;
		}
		//! Returns TRUE if thread should be automatically started on subsystem's start operation
		inline bool autoStart() const
		{
			return _autoStart;
		}
		//! Returns TRUE if the thread will be terminated and joined to on subsytem's stop operation
		inline bool autoStop() const
		{
			return _autoStop;
		}
		//! Returns TRUE if the thread should be terminated due to it's subsystem state
		/*!
		  This method should be periodically called from the AbstractThread::run() to terminate thread execution when needed.
		*/
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
		Subsystem& _subsystem;
		bool _autoStart;
		bool _autoStop;
		bool _shouldTerminate;
		WaitCondition _shouldTerminateCond;
	};
	//! Start/stop operations locker utility class
	class StartStopLocker
	{
	public:
		StartStopLocker(Subsystem& subsystem) :
			_locker(subsystem._startStopMutex)
		{}
	private:
		StartStopLocker();
		StartStopLocker(const StartStopLocker&);

		StartStopLocker& operator=(const StartStopLocker&);

		MutexLocker _locker;
	};
	//! Before start event handler
	virtual void beforeStart()
	{}
	//! Starts children and threads
	virtual void startImpl();
	//! After start event handler
	virtual void afterStart()
	{}
	//! Before stop event handler
	virtual void beforeStop()
	{}
	//! Stops children and threads
	virtual void stopImpl();
	//! After stop event handler
	virtual void afterStop()
	{}
	//! Subsystem's runtime parameters R/W-lock
	/*!
	  Use it for thread-safely locking of any of your subsystem's runtime parameter.
	  This memeber has been introduced in order to save system resources by using the same
	  R/W-lock for all runtime parameters of the subsystem.
	*/
	mutable ReadWriteLock runtimeParamsRWLock;
private:
	Subsystem();
	Subsystem(const Subsystem&);							// No copy

	Subsystem& operator=(const Subsystem&);						// No copy

	typedef std::list<Subsystem *> Children;
	typedef std::list<AbstractThread *> Threads;

	void registerChild(Subsystem * child);
	void unregisterChild(Subsystem * child);
	void registerThread(AbstractThread * thread);
	void unregisterThread(AbstractThread * thread);

	Subsystem * _owner;
	Children _children;
	Threads _threads;
	Mutex _startStopMutex;
};

} // namespace isl

#endif

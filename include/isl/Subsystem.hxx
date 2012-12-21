#ifndef ISL__SUBSYSTEM__HXX
#define ISL__SUBSYSTEM__HXX

#include <isl/ReadWriteLock.hxx>
#include <isl/Mutex.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Debug.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/AbstractThread.hxx>
#include <isl/InterThreadRequester.hxx>
#include <isl/InterThreadMessage.hxx>
#include <string>
#include <algorithm>

#ifndef ISL__SUBSYSTEM_DEFAULT_AWAIT_RESPONSE_TIMEOUT_RATIO
#define ISL__SUBSYSTEM_DEFAULT_AWAIT_RESPONSE_TIMEOUT_RATIO 10
#endif

namespace isl
{

//! Server subsystem base class
/*!
  Basic component of any server according to <a href="http://en.wikipedia.org/wiki/Composite_pattern">Composite Design Pattern</a>.
*/
class Subsystem
{
public:
	enum Constants {
		DefaultAwaitResponseTimeoutRatio = ISL__SUBSYSTEM_DEFAULT_AWAIT_RESPONSE_TIMEOUT_RATIO
	};

	typedef InterThreadRequester<AbstractInterThreadMessage, CloneMessageCloner<AbstractInterThreadMessage> > InterThreadRequesterType;
	//! Constructs a new subsystem
	/*!
	  \param owner The owner subsystem of the new subsystem
	  \param clockTimeout Subsystem's clock timeout
	  \param awaitResponseTimeoutRatio Amount of clock timeouts to wait for the inter-thread response
	 */
	Subsystem(Subsystem * owner, const Timeout& clockTimeout = Timeout::defaultTimeout(),
			size_t awaitResponseTimeoutRatio = DefaultAwaitResponseTimeoutRatio);
	//! Destructor
	virtual ~Subsystem();
	//! Returns an owner of the subsystem
	/*!
	  \note Thread-safe
	*/
	inline Subsystem * owner() const
	{
		return _owner;
	}
	//! Returns clock timeout
	inline const Timeout& clockTimeout() const
	{
		return _clockTimeout;
	}
	//! Sets new clock timeout
	/*!
	  \param newValue New clock timeout value

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	inline void setClockTimeout(const Timeout& newValue)
	{
		_clockTimeout = newValue;
	}
	//! Returns amount of clock timeouts to wait for the inter-thread response
	inline size_t awaitResponseTimeoutRatio() const
	{
		return _awaitResponseTimeoutRatio;
	}
	//! Sets new amount of clock timeouts to wait for the inter-thread response
	/*!
	  \param newValue New amount of clock timeouts to wait for the inter-thread response

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	inline void setAwaitResponseTimeoutRatio(size_t newValue)
	{
		_awaitResponseTimeoutRatio = newValue;
	}
	//! Returns await for inter-thread response timeout
	inline Timeout awaitResponseTimeout() const
	{
		return _clockTimeout * _awaitResponseTimeoutRatio;
	}
	//! Starting subsystem virtual method
	/*!
	  Default implementation starts all children subsystems and subsystem's threads
	  \note Thread-unsafe
	*/
	virtual void start();
	//! Stopping subsystem and awaiting for it's termination virtual method
	/*!
	  Default implementation stops all subsystem's threads and children subsystems
	  \note Thread-unsafe
	*/
	virtual void stop();
protected:
	//! Subsystem-controllable abstract thread
	class AbstractThread : public ::isl::AbstractThread
	{
	public:
		//! Constructs subsystem-controllable thread, which is controlled by the inter-thread 
		/*!
		  \param subsystem Reference to subsystem object new thread is controlled by
		  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
		  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
		*/
		AbstractThread(Subsystem& subsystem, bool isTrackable = false, bool awaitStartup = false);
		//! Destructor
		virtual ~AbstractThread();
		//! Returns a reference to the subsystem object
		inline Subsystem& subsystem() const
		{
			return _subsystem;
		}
		//! Returns a reference to the inter-thread requester object
		inline InterThreadRequesterType& requester()
		{
			return _requester;
		}
	private:
		Subsystem& _subsystem;
		InterThreadRequesterType _requester;
	};
	//! Starts children subsystems
	void startChildren();
	//! Stops children subsystems
	void stopChildren();
	//! Starts subsystem's threads
	void startThreads();
	//! Stops subsystem's threads
	void stopThreads();
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
	Timeout _clockTimeout;
	size_t _awaitResponseTimeoutRatio;
	Children _children;
	Threads _threads;
};

} // namespace isl

#endif

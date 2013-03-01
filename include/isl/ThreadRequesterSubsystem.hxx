#ifndef ISL__THREAD_REQUESTER_SUBSYSTEM__HXX
#define ISL__THREAD_REQUESTER_SUBSYSTEM__HXX

#include <isl/Subsystem.hxx>
#include <isl/AbstractThread.hxx>
#include <isl/ThreadRequester.hxx>

namespace isl
{

//! Subsystem, which is controlling it's threads using their internal ThreadRequester objects
class ThreadRequesterSubsystem : public Subsystem
{
public:
	//! Constructs a new thread requester subsystem
	/*!
	  \param owner The owner subsystem of the new subsystem
	  \param clockTimeout Subsystem's clock timeout
	  \param awaitResponseTimeout Timeout to await for the response to thread request
	 */
	ThreadRequesterSubsystem(Subsystem * owner, const Timeout& clockTimeout = Timeout::defaultTimeout(), const Timeout& awaitResponseTimeout = Timeout::defaultTimeout() * 4);
	//! Returns timeout to await for the response to thread request
	inline const Timeout& awaitResponseTimeout() const
	{
		return _awaitResponseTimeout;
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
	//! Abstract inter-thread message
	class AbstractThreadMessage
	{
	public:
		virtual ~AbstractThreadMessage()
		{}
		//! Inspects inter-thread message to be instance of a class
		/*!
		  \tparam T Class to inspect for
		  \return TRUE if an inter-thread message is an instance of a class
		*/
		template <typename T> bool instanceOf() const
		{
			return dynamic_cast<const T *>(this);
		}
		//! Casts inter-thread message
		/*!
		  \tparam T Class to cast to
		  \return Pointer to casted inter-thread message or to zero if typecasting is not possible
		*/
		template <typename T> T * cast() const
		{
			return dynamic_cast<T *>(this);
		}
		//! Returns inter-thread message name
		virtual const char * name() const = 0;
		//! Clones inter-thread message
		/*!
		  \return Pointer to new cloned inter-thread message
		*/
		virtual AbstractThreadMessage * clone() const = 0;
	private:
	};
	typedef ThreadRequester<AbstractThreadMessage, CloneMessageCloner<AbstractThreadMessage> > ThreadRequesterType;

	//! Termination request inter-thread message
	class TerminationRequest : public AbstractThreadMessage
	{
	public:
		virtual const char * name() const
		{
			static const char * n = "Termination request";
			return n;
		}
		virtual AbstractThreadMessage * clone() const
		{
			return new TerminationRequest(*this);
		}
	};

	//! OK response inter-thread message
	class OkResponse: public AbstractThreadMessage
	{
	public:
		virtual const char * name() const
		{
			static const char * n = "OK response";
			return n;
		}
		virtual AbstractThreadMessage * clone() const
		{
			return new OkResponse(*this);
		}
	};

	//! Thread requester subsystem controllable abstract thread
	class AbstractThread : public ::isl::AbstractThread
	{
	public:
		//! Constructs thread requester subsystem controllable thread
		/*!
		  \param subsystem Reference to state set subsystem object new thread is controlled by
		  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
		  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
		*/
		AbstractThread(ThreadRequesterSubsystem& subsystem, bool isTrackable = false, bool awaitStartup = false);
		//! Destructor
		virtual ~AbstractThread();
		//! Returns a reference to the state set subsystem
		inline ThreadRequesterSubsystem& subsystem()
		{
			return _subsystem;
		}
		//! Returns a reference to the thread requester
		inline ThreadRequesterType& requester()
		{
			return _requester;
		}
	protected:
		//! Returns TRUE if the thread should be terminated
		inline bool shouldTerminate() const
		{
			return _shouldTerminate;
		}
		//! Processes all pending thread requests
		void processThreadRequests();
		//! Awaits for pending thread requests and processes them until limit timestamp has been reached
		/*!
		  \param limit Limit timestamp to await for thread requests
		*/
		void processThreadRequests(const Timestamp& limit);

		//! On thread request event handler
		/*!
		  \param pendingRequest Constant reference to pending resuest to process
		*/
		virtual void onThreadRequest(const ThreadRequesterType::PendingRequest& pendingRequest)
		{}
	private:
		//! Processes thread request
		/*!
		  \param pendingRequest Constant reference to pending resuest to process
		*/
		void processThreadRequest(const ThreadRequesterType::PendingRequest& pendingRequest);

		ThreadRequesterSubsystem& _subsystem;
		ThreadRequesterType _requester;
		bool _shouldTerminate;

		friend class ThreadRequesterSubsystem;
	};

	//! Thread requester subsystem controllable thread with main loop
	class Thread : public AbstractThread
	{
	public:
		//! Constructs thread requester subsystem controllable thread
		/*!
		  \param subsystem Reference to state set subsystem object new thread is controlled by
		  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
		  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
		*/
		Thread(ThreadRequesterSubsystem& subsystem, bool isTrackable = false, bool awaitStartup = false);
		//! On start event handler
		/*!
		  Default implementation does nothing and returns TRUE.
		  \return TRUE if to continue thread execution
		*/
		virtual bool onStart()
		{
			return true;
		}
		//! Doing the work virtual method
		/*!
		  Default implementation does nothing and returns TRUE.
		  \param limit Limit timestamp for doing the work
		  \return TRUE if to continue thread execution
		*/
		virtual bool doLoad(const Timestamp& limit)
		{
			return true;
		}
		//! On overload event handler
		/*!
		  Default implementation does nothing and returns TRUE.
		  \param Ticks expired (always > 2)
		  \return TRUE if to continue thread execution
		*/
		virtual bool onOverload(size_t ticksExpired)
		{
			return true;
		}
		//! On stop event handler
		virtual void onStop()
		{}
	private:
		//! Thread execution virtual method redefinition
		virtual void run();
	};
private:
	typedef std::list<AbstractThread *> Threads;

	void registerThread(AbstractThread * thread);
	void unregisterThread(AbstractThread * thread);

	Timeout _awaitResponseTimeout;
	Threads _threads;
};

} // namespace isl

#endif

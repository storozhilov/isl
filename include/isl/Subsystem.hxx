#ifndef ISL__SUBSYSTEM__HXX
#define ISL__SUBSYSTEM__HXX

#include <isl/Timeout.hxx>
#include <isl/Thread.hxx>
#include <isl/ThreadRequester.hxx>
#include <isl/BasicDateTime.hxx>
#include <list>

#ifndef ISL__DEFAULT_SUBSYSTEM__AWAIT_RESPONSE_TICKS_AMOUNT
#define ISL__DEFAULT_SUBSYSTEM__AWAIT_RESPONSE_TICKS_AMOUNT 4
#endif

namespace isl
{

//! Server subsystem base class
/*!
  Basic component of any server according to a <a href="http://en.wikipedia.org/wiki/Composite_pattern">Composite Design Pattern</a>.
  Subsystem could contain and performs a control on it's threads and children subsystems.
 */ 
class Subsystem
{
public:
	enum Constants {
		//! Default amount of clock ticks to await response from the another thread
		DefaultAwaitResponseTicksAmount = ISL__DEFAULT_SUBSYSTEM__AWAIT_RESPONSE_TICKS_AMOUNT
	};
	//! Abstract thread, which is controllable by a subsystem
	class AbstractThread
	{
	public:
		//! Constructs a thread
		/*!
		  \param subsystem Reference to the subsystem object a thread is to be controlled by
		  \param isTrackable If TRUE thread().isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
		  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
		*/
		AbstractThread(Subsystem& subsystem, bool isTrackable = false, bool awaitStartup = false) :
			_subsystem(subsystem),
			_thread(isTrackable, awaitStartup)
		{
			_subsystem.registerThread(this);
		}
		virtual ~AbstractThread()
		{
			_subsystem.unregisterThread(this);
		}
		//! Returns a reference to the subsystem
		inline Subsystem& subsystem()
		{
			return _subsystem;
		}

		//! Appoints a subsystem's thread termination
		/*!
		 * Could be called from this thread or outside, so proper work has to be provided in descendants.
		 * Call it when you whant to terminate thread execution. Actually, the subsystem will do it itself,
		 * so use it when you are fully aware of what are you doing.
		 * \note Thread-safe
		 */
		virtual void appointTermination() = 0;
	protected:
		//! Returns a reference to the thread object
		inline Thread& thread()
		{
			return _thread;
		}

		//! Starts thread execution
		void start()
		{
			reset();
			_thread.start(*this, &AbstractThread::execute);
		}

		//! Resets thread (called before start)
		/*!
		  \note Thread-unsafe
		*/
		virtual void reset() = 0;
		//! Thread execution abstract virtual method to override in subclasses
		virtual void run() = 0;
		//! Returns TRUE if the thread should be terminated
		virtual bool shouldTerminate() = 0;
	private:
		Subsystem& _subsystem;
		Thread _thread;

		inline void execute()
		{
			run();
		}

		friend class Subsystem;
	};
	//! Abstract inter-thread message
	class AbstractThreadMessage
	{
	public:
		//! Constructs inter-thread message
		/*!
		 * \param name Name of the message type
		 */
		AbstractThreadMessage(const std::string& name) :
			_name(name)
		{}
		virtual ~AbstractThreadMessage()
		{}
		//! Returns inter-thread message name
		inline const std::string& name() const
		{
			return _name;
		}
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
		//! Clones inter-thread message
		/*!
		  \return Pointer to new copy of the inter-thread message
		*/
		virtual AbstractThreadMessage * clone() const = 0;
	private:
		AbstractThreadMessage();

		const std::string _name;
	};
	typedef ThreadRequester<AbstractThreadMessage, CloneMessageCloner<AbstractThreadMessage> > ThreadRequesterType;

	//! Ping request inter-thread message
	/*!
	 * Use to check if subsystem is responding by PongResponse to this request
	 * \sa PongResponse
	 */
	class PingRequest : public AbstractThreadMessage
	{
	public:
		PingRequest() :
			AbstractThreadMessage("Ping request")
		{}

		virtual AbstractThreadMessage * clone() const
		{
			return new PingRequest(*this);
		}
	};

	//! Pong response inter-thread message
	/*!
	 * A thread is responding to PingRequest using this response
	 * \sa PingRequest
	 */
	class PongResponse : public AbstractThreadMessage
	{
	public:
		PongResponse() :
			AbstractThreadMessage("Pong response")
		{}

		virtual AbstractThreadMessage * clone() const
		{
			return new PongResponse(*this);
		}
	};

	//! Termination request inter-thread message
	class TerminationRequest : public AbstractThreadMessage
	{
	public:
		TerminationRequest() :
			AbstractThreadMessage("Termination request")
		{}

		virtual AbstractThreadMessage * clone() const
		{
			return new TerminationRequest(*this);
		}
	};

	//! OK response inter-thread message
	class OkResponse: public AbstractThreadMessage
	{
	public:
		OkResponse() :
			AbstractThreadMessage("OK response")
		{}

		virtual AbstractThreadMessage * clone() const
		{
			return new OkResponse(*this);
		}
	};

	//! Thread requester controllable abstract thread
        /*!
         * TODO: Migration to RequestableThread class
         */
	class AbstractRequesterThread : public AbstractThread
	{
	public:
		//! Constructs thread requester controllable abstract thread
		/*!
		  \param subsystem Reference to the subsystem object new thread is controlled by
		  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
		  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
		*/
		AbstractRequesterThread(Subsystem& subsystem, bool isTrackable = false, bool awaitStartup = false);
		//! Destructor
		virtual ~AbstractRequesterThread();
		//! Sends request to the thread
		/*!
		  \param request Request message to send
		  \param limit Limit to await for the response
		  \return Auto-pointer to the response or to 0 if no response has been provided
		  \note Could be called from this thread or outside - 'awaitResponseLimit' argument is ignored in first case.
		*/
		std::auto_ptr<ThreadRequesterType::MessageType> sendRequest(const ThreadRequesterType::MessageType& request,
				const Timestamp& awaitResponseLimit);

		//! Resets thread (called before start)
		/*!
		  \note Thread-unsafe
		*/
		virtual void reset()
		{
			_requester.reset();
			_shouldTerminate = false;
		}
		//! Appoints a subsystem's thread termination
		virtual void appointTermination();
	protected:
		//! Returns a reference to the thread requester
		/*!
		 * TODO: Remove it?
		 */
		inline ThreadRequesterType& requester()
		{
			return _requester;
		}
		//! Processes all pending thread requests
		void processRequests();
		//! Awaits for pending thread requests and processes them until limit timestamp has been reached
		/*!
		  \param limit Limit timestamp to await for thread requests
		*/
		void processRequests(const Timestamp& limit);

		//! Returns TRUE if the thread should be terminated
		virtual bool shouldTerminate()
		{
			return _shouldTerminate;
		}
		//! On thread request event handler
		/*!
		  \note Default implementation writes an "unrecognized request" entry in the error log
		  \param request Constant reference to pending request to process
                  \param stopRequestsProcessing A reference to flag, which means to terminate next incoming requests processing [OUT]
		  \return Auto-pointer to the response or to 0 if no response has been provided
		*/
		virtual std::auto_ptr<ThreadRequesterType::MessageType> onRequest(const ThreadRequesterType::MessageType& request, bool responseRequired,
                                bool& stopRequestsProcessing);
	private:
		//! Processes thread request
		/*!
		  \param request Constant reference to pending resuest to process
                  \param stopRequestsProcessing A reference to flag, which means to terminate next incoming requests processing [OUT]
		  \return Auto-pointer to the response or to 0 if no response has been provided
		*/
		std::auto_ptr<ThreadRequesterType::MessageType> processRequest(const ThreadRequesterType::MessageType& request, bool responseRequired,
                                bool& stopRequestsProcessing);

		ThreadRequesterType _requester;
		bool _shouldTerminate;
	};

	//! Thread requester controllable thread with main loop
        /*!
         * TODO: Migration to RequestableThread class
         */
	class RequesterThread : public AbstractRequesterThread
	{
	public:
		//! Constructs thread requester controllable thread
		/*!
		  \param subsystem Reference to the subsystem object new thread is controlled by
		  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
		  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
		*/
		RequesterThread(Subsystem& subsystem, bool isTrackable = false, bool awaitStartup = false);
        protected:
		//! On start event handler
		virtual void onStart()
		{}
		//! Doing the work virtual method
		/*!
		  \param prevTickTimestamp Previous tick timestamp
		  \param nextTickTimestamp Next tick timestamp
		  \param ticksExpired Amount of expired ticks - if > 1, then an overload has occured
		*/
		virtual void doLoad(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
		{}
		//! On overload event handler
		/*!
		  Default implementation does nothing and returns TRUE.
		  \param prevTickTimestamp Previous tick timestamp
		  \param nextTickTimestamp Next tick timestamp
		  \param Amount of expired ticks - always > 2
		*/
		virtual void onOverload(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
		{}
		//! On stop event handler
		virtual void onStop()
		{}
	private:
		//! RequesterThread execution virtual method redefinition
		virtual void run();
	};

        //! Requestable thread with main loop
        /*!
         * A thread which is periodically doing the load cycle, checks out for incoming requests
         * and handles them.
         * TODO: Migration to this class
         */
        class RequestableThread : public AbstractThread
        {
        public:
		//! Constructs thread requester controllable abstract thread
		/*!
		  \param subsystem Reference to the subsystem object new thread is controlled by
		  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
		  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
		*/
		RequestableThread(Subsystem& subsystem, bool isTrackable = false, bool awaitStartup = false);
		//! Destructor
		virtual ~RequestableThread();
		//! Sends request to the thread
		/*!
		  \param request Request message to send
		  \param limit Limit to await for the response
		  \return Auto-pointer to the response or to 0 if no response has been provided
		  \note Could be called from this thread or outside - 'awaitResponseLimit' argument is ignored in first case.
		*/
		std::auto_ptr<ThreadRequesterType::MessageType> sendRequest(const ThreadRequesterType::MessageType& request,
				const Timestamp& awaitResponseLimit);

		//! Resets thread (called before start)
		/*!
		  \note Thread-unsafe
		*/
		virtual void reset()
		{
			_requester.reset();
			_shouldTerminate = false;
		}
		//! Appoints a subsystem's thread termination
		virtual void appointTermination();
        protected:
		//! Returns a reference to the thread requester
		/*!
		 * TODO: Remove it?
		 */
		inline ThreadRequesterType& requester()
		{
			return _requester;
		}
		//! Processes all pending thread requests
		void processRequests();
		//! Awaits for pending thread requests and processes them until limit timestamp has been reached
		/*!
		  \param limit Limit timestamp to await for thread requests
		*/
		void processRequests(const Timestamp& limit);
                //! Returns a next load cycle timestamp
                const Timestamp& nextLoad() const;
                //! Appoints a load cycle
                void appointLoad(const Timeout& timeout);

		//! Returns TRUE if the thread should be terminated
		virtual bool shouldTerminate()
		{
			return _shouldTerminate;
		}
		//! On thread request event handler
		/*!
		  \note Default implementation writes an "unrecognized request" entry in the error log
		  \param request Constant reference to pending request to process
                  \param stopRequestsProcessing A reference to flag, which means to terminate next incoming requests processing [OUT]
		  \return Auto-pointer to the response or to 0 if no response has been provided
		*/
		virtual std::auto_ptr<ThreadRequesterType::MessageType> onRequest(const ThreadRequesterType::MessageType& request, bool responseRequired,
                                bool& stopRequestsProcessing);
		//! On start event handler
		virtual void onStart()
		{}
		//! Doing the load cycle virtual method
		/*!
                  Default implementation does nothing and returns next year timestamp
		  \param prevTimestamp Previous load cycle timestamp
                  \param appointedTimestamp Timestamp this load cycle execution was appointed at
		  \param limitTimestamp Limit timestamp for the load cycle
                  \return Next load cycle timestamp
		*/
		virtual Timestamp doLoad(const Timestamp& prevTimestamp, const Timestamp& appointedTimestamp, const Timestamp& limitTimestamp)
		{
                        return limitTimestamp + Timeout(BasicDateTime::SecondsPerDay * 365);
                }
		//! On overload event handler
		/*!
                  Called if a load cycle was executed longer than clock timeout of the subsystem
		  Default implementation does nothing and returns TRUE.
		  \param limitTimestamp Load cycle limit timestamp
		  \param actualTimestamp Load cycle end of execution timestamp
		*/
		virtual void onOverload(const Timestamp& limitTimestamp, const Timestamp& actualTimestamp)
		{}
		//! On stop event handler
		virtual void onStop()
		{}
	private:
		//! Processes thread request
		/*!
		  \param request Constant reference to pending resuest to process
                  \param stopRequestsProcessing A reference to flag, which means to terminate next incoming requests processing [OUT]
		  \return Auto-pointer to the response or to 0 if no response has been provided
		*/
		std::auto_ptr<ThreadRequesterType::MessageType> processRequest(const ThreadRequesterType::MessageType& request, bool responseRequired,
                                bool& stopRequestsProcessing);

		//! RequesterThread execution virtual method redefinition
		virtual void run();

		ThreadRequesterType _requester;
		bool _shouldTerminate;
        };

	//! Constructs a new subsystem
	/*!
	  \param owner The owner subsystem of the new subsystem
	  \param clockTimeout Subsystem's clock timeout (TODO: Set default to Subsystem::defaultClockTimeout())
	  \param awaitResponseTicksAmount Amount of clock ticks to await response from the another thread
	 */
	Subsystem(Subsystem * owner, const Timeout& clockTimeout = Timeout::defaultTimeout(),
			size_t awaitResponseTicksAmount = DefaultAwaitResponseTicksAmount);
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
	//! Returns amount of clock ticks to await response from the another thread
	inline size_t awaitResponseTicksAmount() const
	{
		return _awaitResponseTicksAmount;
	}
	//! Returns a timeout to await for the response from the any thread of the subsystem
	inline const Timeout awaitResponseTimeout() const
	{
		return _clockTimeout * _awaitResponseTicksAmount;
	}
	//! Sets amount of clock ticks to await response from the another thread
	/*!
	 * \param newValue New amount of clock ticks to await response from the another thread
	 * \note Thread-unsafe: call it when subsystem is idling only
	 */
	inline void setAwaitResponseTicksAmount(size_t newValue)
	{
		_awaitResponseTicksAmount = newValue;
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
	//! Starts children subsystems
	void startChildren();
	//! Stops children subsystems
	void stopChildren();
	//! Starts state set subsystem's threads
	void startThreads();
	//! Stops state set subsystem's threads
	void stopThreads();
private:
	Subsystem();
	Subsystem(const Subsystem&);							// No copy

	Subsystem& operator=(const Subsystem&);						// No copy

	typedef std::list<Subsystem *> Children;

	void registerChild(Subsystem * child);
	void unregisterChild(Subsystem * child);

	typedef std::list<AbstractThread *> Threads;

	void registerThread(AbstractThread * thread);
	void unregisterThread(AbstractThread * thread);

	Subsystem * _owner;
	Timeout _clockTimeout;
	size_t _awaitResponseTicksAmount;
	Children _children;
	Threads _threads;
};

} // namespace isl

#endif

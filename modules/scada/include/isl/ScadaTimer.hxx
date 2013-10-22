#ifndef ISL__SCADA_TIMER__HXX
#define ISL__SCADA_TIMER__HXX

#include <isl/Timer.hxx>
#include <isl/ReadWriteLock.hxx>
#include <set>

namespace isl
{

class ScadaProgram;

//! Timer to run SCADA programs
class ScadaTimer : public Timer
{
public:
	//! Constructs a SCADA timer
	/*!
	  \param owner Pointer to owner subsystem
	  \param clockTimeout Timer clock timeout
	  \param maxScheduledTasksAmount Maximum amount of scheduled tasks
	*/
	ScadaTimer(Subsystem * owner, const Timeout& clockTimeout = Timeout::defaultTimeout(),
			size_t maxScheduledTasksAmount = DefaultMaxScheduledTasksAmount);

	//! Registers SCADA program in SCADA timer
	/*!
	 * \param program Program to be registered
	 * \note Thread-unsafe: call it when subsystem is idling only
	 */
	void registerProgram(ScadaProgram& program);
	//! Unregisters SCADA program from SCADA timer
	/*!
	 * \param program Program to be unregistered
	 * \note Thread-unsafe: call it when subsystem is idling only
	 */
	void unregisterProgram(ScadaProgram& program);
protected:
	//! SCADA timer thread class
	class ScadaTimerThread : public TimerThread
	{
	public:
		//! Creates a SCADA timer thread
		/*!
		 * \param scadaTimer A reference to SCADA timer object
		 */
		ScadaTimerThread(ScadaTimer& scadaTimer);

		//! On start event handler
		/*!
		  Default implementation does nothing and returns TRUE.
		  \return TRUE if to continue thread execution
		*/
		virtual void onStart();
		//! On stop event handler
		virtual void onStop();
		//! On thread request event handler
		/*!
		  \param request Constant reference to pending request to process
                  \param stopRequestsProcessing A reference to flag, which means to terminate next incoming requests processing [OUT]
		  \return Auto-pointer to the response or to 0 if no response has been provided
		*/
		virtual std::auto_ptr<AbstractThreadMessage> onRequest(const AbstractThreadMessage& request, bool responseRequired,
                                bool& stopRequestsProcessing);
	private:
		ScadaTimerThread();
		ScadaTimerThread(const ScadaTimerThread&);							// No copy

		ScadaTimerThread& operator=(const ScadaTimerThread&);						// No copy

		ScadaTimer& _scadaTimer;
	};

	//! Envelop for the SCADA program message
	class ScadaProgramMessageEnvelope : public AbstractThreadMessage
	{
	public:
		ScadaProgramMessageEnvelope(ScadaProgram& program, const AbstractThreadMessage& message);
		ScadaProgramMessageEnvelope(const ScadaProgramMessageEnvelope& other);

		ScadaProgramMessageEnvelope& operator=(const ScadaProgramMessageEnvelope& rhs);

		virtual AbstractThreadMessage * clone() const
		{
			return new ScadaProgramMessageEnvelope(*this);
		}
	private:
		ScadaProgram * _program;
		std::auto_ptr<AbstractThreadMessage> _messageAutoPtr;

		friend class ScadaTimerThread;
	};

	//! Timer thread creation factory method
	/*!
	 * \returns A pointer to new timer thread object
	 */
	virtual TimerThread * createThread()
	{
		//return new ScadaTimerThread(*this);
		_scadaTimerThreadPtr = new ScadaTimerThread(*this);
		return _scadaTimerThreadPtr;
	}
private:
	typedef std::set<ScadaProgram *> ProgramsContainer;

	ScadaTimerThread * _scadaTimerThreadPtr;
	ProgramsContainer _programs;
	ReadWriteLock _isRunningRwLock;
	bool _isRunning;

	friend class ScadaProgram;
};

} // namespace isl

#endif

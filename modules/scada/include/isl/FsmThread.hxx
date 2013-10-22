#ifndef ISL__FINITE_STATE_MACHINE_THREAD__HXX
#define ISL__FINITE_STATE_MACHINE_THREAD__HXX

#include <isl/Subsystem.hxx>

namespace isl
{

//! Finite-State Machine thread
/*!
 * TODO: To subclass it from Subsystem::SchedulerThread class
 */
class FsmThread : public Subsystem::AbstractRequestableThread
{
public:
        class AbstractState;
        //! Next step data: pointer to next step and timeout to make next step after
        //typedef std::pair<AbstractState *, Timeout> NextStep;
        typedef std::pair<AbstractState&, Timeout> NextStep;

        //! Finite-state machine state
        class AbstractState
        {
        public:
                //! Constructs a finite-state machine state
                /*!
                 * \param name A name of the state
                 */
                AbstractState(const std::string& name) :
                        _name(name)
                {}
                virtual ~AbstractState()
                {}

                //! Returns a state name
                inline const std::string& name() const
                {
                        return _name;
                }
                //! Returns true if a state is an instance of particular type
                template <typename T> bool instanceOf() const
                {
                        return dynamic_cast<T *>(this);
                }
                //! Casts a state and returns a pointer to it
                template <typename T> T * cast() const
                {
                        return dynamic_cast<T *>(this);
                }

                //! Makes next step
                /*!
                 * \param fsmThread A reference to Finite-State Machine thread
                 * \returns A pair with a pointer to the next step and a timeout to make next step after
                 */
                virtual NextStep makeStep(FsmThread& fsmThread) = 0;
        private:
                AbstractState();
                AbstractState(const AbstractState&);

                AbstractState& operator=(const AbstractState&);

                const std::string _name;
        };

        //! Constructs a Finite-State Machine thread
        /*!
          \param subsystem Reference to the subsystem object new thread is controlled by
          \param initialState A reference to initial state-machine thread
          \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
          \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
        */
        FsmThread(Subsystem& subsystem, AbstractState& initialState, bool isTrackable = false, bool awaitStartup = false);
protected:
        //! Appoints a next state
        void appointNextState(const NextStep& nextStep);
        //! On start event handler
        virtual void onStart()
        {}
        //! Doing the work virtual method
        /*!
          \param prevTickTimestamp Previous tick timestamp
          \param nextTickTimestamp Next tick timestamp
          \param ticksExpired Amount of expired ticks - if > 1, then an overload has occured
        */
//        virtual void doLoad(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
//        {}
        //! On overload event handler
        /*!
          Default implementation does nothing and returns TRUE.
          \param prevTickTimestamp Previous tick timestamp
          \param nextTickTimestamp Next tick timestamp
          \param Amount of expired ticks - always > 2
        */
//        virtual void onOverload(const Timestamp& prevTickTimestamp, const Timestamp& nextTickTimestamp, size_t ticksExpired)
//        {}
        //! On stop event handler
        virtual void onStop()
        {}
private:
        //! RequesterThread execution virtual method redefinition
        virtual void run();
private:
        AbstractState& _initialState;
        AbstractState * _nextStepStatePtr;
        Timestamp _nextStepTimestamp;
};

} // namespace isl

#endif

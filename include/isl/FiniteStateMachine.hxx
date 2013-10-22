#ifndef ISL__FINITE_STATE_MACHINE__HXX
#define ISL__FINITE_STATE_MACHINE__HXX

#include <isl/Subsystem.hxx>

namespace isl
{

//! A class with a couple of finite-state-machine thread classes
/*!
 * \note Experimental
 */
class FiniteStateMachine : public Subsystem
{
public:
	//! A finite-state machine thread which checks for requests periodically
        /*!
         * It tries to use all load cycle timespan to run a finite-state machine
         */
        class OscillatorThread : public Subsystem::OscillatorThread
        {
        public:
                //! Oscillative finite-state machine state
                class AbstractState
                {
                public:
                        //! Constructs an oscillative finite-state machine state
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
                         * \param limit Limit timestamp to make step
                         * \returns A reference to the next state
                         */
                        virtual AbstractState& makeStep(const Timestamp& limit) = 0;
                private:
                        AbstractState();
                        AbstractState(const AbstractState&);

                        AbstractState& operator=(const AbstractState&);

                        const std::string _name;
                };

                //! Constructs a oscillator finite-state machine thread
                /*!
                  \param subsystem Reference to the subsystem object new thread is controlled by
                  \param initialState A reference to initial state-machine thread
                  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
                  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
                */
                OscillatorThread(Subsystem& subsystem, AbstractState& initialState, bool isTrackable = false, bool awaitStartup = false);

                //! Returns a reference to the initial state
                inline const AbstractState& initialState() const
                {
                        return _initialState;
                }
        protected:
                //! Appoints a next state
                /*!
                 * \param state A reference to state to appoint
                 */
                void appointState(AbstractState& state);

		//! On start event handler
		virtual void onStart();
		//! Doing the load cycle virtual method
		/*!
		  \param prevTick Previous tick timestamp
		  \param nextTick Next tick timestamp
		  \param ticksExpired Amount of expired ticks - if > 1, then an overload has occured
		*/
		virtual void doLoad(const Timestamp& prevTick, const Timestamp& nextTick, size_t ticksExpired);
        private:
                AbstractState& _initialState;
                AbstractState * _nextStepStatePtr;
        };

        //! A finite-state machine thread which schedules state for itself
        /*!
         * Most relaxed finite-state machine driver. It tries to sleep on thread requester
         * conditional variable all possible time.
         */
        class SchedulerThread : public Subsystem::SchedulerThread
        {
        public:
                class AbstractState;
                //! Next step data: Reference to next step => timeout to make next step after
                typedef std::pair<AbstractState&, Timeout> NextStep;

                //! Scheduling finite-state machine state
                class AbstractState
                {
                public:
                        //! Constructs a scheduling finite-state machine state
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
                         * \param limit Limit timestamp to make step
                         * \returns A pair with a reference to the next step and a timeout to make next step after
                         */
                        virtual NextStep makeStep(const Timestamp& limit) = 0;
                private:
                        AbstractState();
                        AbstractState(const AbstractState&);

                        AbstractState& operator=(const AbstractState&);

                        const std::string _name;
                };

                //! Constructs a scheduler finite-state machine thread
                /*!
                  \param subsystem Reference to the subsystem object new thread is controlled by
                  \param initialState A reference to initial state-machine thread
                  \param isTrackable If TRUE isRunning() method could be used for inspecting if the thread is running for the cost of R/W-lock
                  \param awaitStartup If TRUE, then launching thread will wait until new thread is started for the cost of condition variable and mutex
                */
                SchedulerThread(Subsystem& subsystem, AbstractState& initialState, bool isTrackable = false, bool awaitStartup = false);

                //! Returns a reference to the initial state
                inline const AbstractState& initialState() const
                {
                        return _initialState;
                }
        protected:
                //! Schedules a next step
                /*!
                 * \param state A reference to state to schedule
                 * \param timestamp Timestamp for the next step
                 */
                void scheduleStep(AbstractState& state, const Timestamp& timestamp);

		//! On start event handler
		virtual void onStart();
		//! Doing the load cycle virtual method
		/*!
                  \param start Timestamp this load cycle execution was scheduled at
		  \param limit Limit timestamp for the load cycle
                  \return Next load cycle timestamp
		*/
		virtual Timestamp doLoad(const Timestamp& start, const Timestamp& limit);
        private:
                AbstractState& _initialState;
                AbstractState * _nextStepStatePtr;
        };
};

} // namespace isl

#endif

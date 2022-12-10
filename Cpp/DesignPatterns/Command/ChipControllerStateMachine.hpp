// This file as been generated the December 09, 2022 from the PlantUML statechart Chip.plantuml
// This code generation is still experimental. Some border cases may not be correctly managed!

#ifndef CHIPCONTROLLER_STATE_MACHINE_HPP
#  define CHIPCONTROLLER_STATE_MACHINE_HPP

#  include "StateMachine.hpp"

//********************************************************************************
//! \brief States of the state machine.
//********************************************************************************
enum class ChipControllerStates
{
    // Client states:
    CONSTRUCTOR,
    BOOTLOADING,
    RUNNING,
    DEGRADED,
    // Mandatory internal states:
    IGNORING_EVENT, CANNOT_HAPPEN, MAX_STATES
};

//********************************************************************************
//! \brief Convert enum states to human readable string.
//********************************************************************************
static inline const char* stringify(ChipControllerStates const state)
{
    static const char* s_states[] =
    {
        [int(ChipControllerStates::CONSTRUCTOR)] = "[*]",
        [int(ChipControllerStates::BOOTLOADING)] = "BOOTLOADING",
        [int(ChipControllerStates::RUNNING)] = "RUNNING",
        [int(ChipControllerStates::DEGRADED)] = "DEGRADED",
    };

    return s_states[int(state)];
};

//********************************************************************************
//! \brief Simulate chip controller.
//! \startuml
//! BOOTLOADING : entering / bootload();
//! [*] --> BOOTLOADING
//! BOOTLOADING --> RUNNING : triggerSuccess\n--\nrunning()
//! BOOTLOADING --> DEGRADED : triggerFailure\n--\ndegraded()
//! RUNNING --> DEGRADED : triggerFailure\n--\ndegraded()
//! \enduml
//********************************************************************************
class ChipControllerStateMachine : public StateMachine<ChipControllerStateMachine, ChipControllerStates>
{
public: // Constructor and destructor

    //----------------------------------------------------------------------------
    //! \brief Default constructor. Start from initial state and call it actions.
    //----------------------------------------------------------------------------
    ChipControllerStateMachine()
        : StateMachine(ChipControllerStates::CONSTRUCTOR)
    {
        // Init actions on states
        m_states[int(ChipControllerStates::BOOTLOADING)] =
        {
            .entering = &ChipControllerStateMachine::onEntering_BOOTLOADING,
        };

        // Init user code
    }

    //----------------------------------------------------------------------------
    //! \brief Needed because of virtual methods.
    //----------------------------------------------------------------------------
    virtual ~ChipControllerStateMachine() = default;

    //---------------------------------------------------------------------------------------
    //! \brief Reset the state machine and nested machines. Do the initial internal transition.
    //---------------------------------------------------------------------------------------
    void enter()
    {
        StateMachine::enter();

        // Internal transition
        {
            LOGD("[CHIPCONTROLLER][STATE [*]] Candidate for internal transitioning to state BOOTLOADING\n");
            static const Transition tr =
            {
                .destination = ChipControllerStates::BOOTLOADING,
            };
            transition(&tr);
        }
    }

    //---------------------------------------------------------------------------------------
    //! \brief Reset the state machine and nested machines. Do the initial internal transition.
    //---------------------------------------------------------------------------------------
    void exit()
    {
        StateMachine::exit();
    }

public: // External events

    //----------------------------------------------------------------------------
    //! \brief External event.
    //----------------------------------------------------------------------------
    void triggerSuccess()
    {
        LOGD("[CHIPCONTROLLER][EVENT %s]\n", __func__);

        // State transition and actions
        static const Transitions s_transitions =
        {
            {
                ChipControllerStates::BOOTLOADING,
                {
                    .destination = ChipControllerStates::RUNNING,
                    .action = &ChipControllerStateMachine::onTransitioning_BOOTLOADING_RUNNING,
                },
            },
        };

        transition(s_transitions);
    }

    //----------------------------------------------------------------------------
    //! \brief External event.
    //----------------------------------------------------------------------------
    void triggerFailure()
    {
        LOGD("[CHIPCONTROLLER][EVENT %s]\n", __func__);

        // State transition and actions
        static const Transitions s_transitions =
        {
            {
                ChipControllerStates::BOOTLOADING,
                {
                    .destination = ChipControllerStates::DEGRADED,
                    .action = &ChipControllerStateMachine::onTransitioning_BOOTLOADING_DEGRADED,
                },
            },
            {
                ChipControllerStates::RUNNING,
                {
                    .destination = ChipControllerStates::DEGRADED,
                    .action = &ChipControllerStateMachine::onTransitioning_RUNNING_DEGRADED,
                },
            },
        };

        transition(s_transitions);
    }

private: // Guards and actions on transitions

    //--------------------------------------------------------------------------------
    //! \brief Do the action when transitioning from state BOOTLOADING to state RUNNING.
    //--------------------------------------------------------------------------------
    MOCKABLE void onTransitioning_BOOTLOADING_RUNNING()
    {
        LOGD("[CHIPCONTROLLER][TRANSITION BOOTLOADING --> RUNNING: running()]\n");
        running();
    }

    //---------------------------------------------------------------------------------
    //! \brief Do the action when transitioning from state BOOTLOADING to state DEGRADED.
    //---------------------------------------------------------------------------------
    MOCKABLE void onTransitioning_BOOTLOADING_DEGRADED()
    {
        LOGD("[CHIPCONTROLLER][TRANSITION BOOTLOADING --> DEGRADED: degraded()]\n");
        degraded();
    }

    //-----------------------------------------------------------------------------
    //! \brief Do the action when transitioning from state RUNNING to state DEGRADED.
    //-----------------------------------------------------------------------------
    MOCKABLE void onTransitioning_RUNNING_DEGRADED()
    {
        LOGD("[CHIPCONTROLLER][TRANSITION RUNNING --> DEGRADED: degraded()]\n");
        degraded();
    }

private: // Actions on states

    //----------------------------------------------------------------------------
    //! \brief Do the action when entering the state BOOTLOADING.
    //----------------------------------------------------------------------------
    MOCKABLE void onEntering_BOOTLOADING()
    {
        LOGD("[CHIPCONTROLLER][ENTERING STATE BOOTLOADING]\n");
        bootload();
    }

private: // Client code

    virtual void bootload() = 0;
    virtual void running() = 0;
    virtual void degraded() = 0;
};

#endif // CHIPCONTROLLER_STATE_MACHINE_HPP

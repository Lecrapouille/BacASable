% ============================================================================
% PROLOG EXPERT SYSTEM - HUMANOID ROBOT DEGRADED MODE MANAGEMENT
% ============================================================================

% ----------------------------------------------------------------------------
% Fact base: Robot component states
% Format: component_state(Component, State).
% Possible states: operational, degraded, failed
% ----------------------------------------------------------------------------

:- dynamic component_state/2.

% Initial configuration (all operational)
component_state(arm_motor, operational).
component_state(hand_hall_sensor, operational).
component_state(shoulder_motor, operational).
component_state(elbow_motor, operational).

% ----------------------------------------------------------------------------
% State query predicates
% ----------------------------------------------------------------------------

operational(Component) :-
    component_state(Component, operational).

failed(Component) :-
    component_state(Component, failed).

degraded(Component) :-
    component_state(Component, degraded).

% Count the number of failures
failure_count(N) :-
    findall(C, failed(C), Failures),
    length(Failures, N).

% ----------------------------------------------------------------------------
% RULE BASE: Conditions for each action
% ----------------------------------------------------------------------------

% R1: Wave hand
% Requires: at least elbow or shoulder motor functional
can_perform(wave_hand) :-
    (operational(elbow_motor) ; operational(shoulder_motor)),
    \+ failed(arm_motor).

% R2: Grasp objects
% Requires: Hall sensor + all motors operational
can_perform(grasp_object) :-
    operational(hand_hall_sensor),
    operational(arm_motor),
    operational(shoulder_motor),
    operational(elbow_motor).

% R3: Move arm
% Requires: arm motor functional
can_perform(move_arm) :-
    operational(arm_motor),
    (operational(shoulder_motor) ; operational(elbow_motor)).

% R4: Lift object
% Requires: all systems operational
can_perform(lift_object) :-
    operational(arm_motor),
    operational(shoulder_motor),
    operational(elbow_motor),
    operational(hand_hall_sensor).

% R5: Point
% Requires: arm motor and shoulder motor
can_perform(point) :-
    operational(arm_motor),
    operational(shoulder_motor).

% R6: Full manipulation
% Requires: all systems operational
can_perform(full_manipulation) :-
    operational(arm_motor),
    operational(shoulder_motor),
    operational(elbow_motor),
    operational(hand_hall_sensor).

% ----------------------------------------------------------------------------
% DIAGNOSTIC RULES
% ----------------------------------------------------------------------------

% Determine severity level
severity_level(nominal) :-
    operational(arm_motor),
    operational(hand_hall_sensor),
    operational(shoulder_motor),
    operational(elbow_motor).

severity_level(light_degradation) :-
    failed(hand_hall_sensor),
    operational(arm_motor),
    operational(shoulder_motor),
    operational(elbow_motor).

severity_level(severe_degradation) :-
    (failed(shoulder_motor) ; failed(elbow_motor)),
    \+ failed(arm_motor),
    failure_count(1).

severity_level(critical) :-
    (failed(arm_motor) ; failure_count(N), N >= 2).

% Explain why an action is not possible
unavailability_reason(wave_hand, 'All arm motors completely failed') :-
    failed(arm_motor).

unavailability_reason(grasp_object, 'Hall effect sensor failed') :-
    failed(hand_hall_sensor).

unavailability_reason(grasp_object, 'Arm motors not operational') :-
    \+ operational(arm_motor).

unavailability_reason(move_arm, 'Arm motor failed') :-
    failed(arm_motor).

unavailability_reason(lift_object, 'Grasping system failed') :-
    (failed(hand_hall_sensor) ; failed(arm_motor)).

% ----------------------------------------------------------------------------
% INTERFACE PREDICATES
% ----------------------------------------------------------------------------

% Update a component's state
update_state(Component, NewState) :-
    retract(component_state(Component, _)),
    assert(component_state(Component, NewState)),
    format('✓ Update: ~w -> ~w~n', [Component, NewState]).

% Reset all components to operational state
reset :-
    retractall(component_state(_, _)),
    assert(component_state(arm_motor, operational)),
    assert(component_state(hand_hall_sensor, operational)),
    assert(component_state(shoulder_motor, operational)),
    assert(component_state(elbow_motor, operational)),
    write('✓ System reset to nominal state'), nl.

% List all possible actions
available_actions(Actions) :-
    setof(Action, can_perform(Action), Actions), !.
available_actions([]).
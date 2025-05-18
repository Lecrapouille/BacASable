#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iomanip>

//=============================================================================
//! \brief Helper function to constrain a value between a minimum and a maximum.
//! \param p_value value to constrain.
//! \param p_min minimum value.
//! \param p_max maximum value.
//=============================================================================
static void constrain(double &p_value, double p_min, double p_max)
{
    p_value = std::min(std::max(p_value, p_min), p_max);
}

//=============================================================================
//! \brief Parameters of the vehicle: what we use to model the vehicle.
//! \param p_mass mass of the vehicle [kg]. Notation: $m$
//! \param p_friction friction coefficient [-]. Notation: $b$
//=============================================================================
struct VehicleParams
{
    VehicleParams(double p_mass = 0.0, double p_friction = 0.0)
        : mass(p_mass), friction(p_friction)
    {}

    //! \brief mass of the vehicle [kg]
    double mass;
    //! \brief friction coefficient [-]
    double friction;
};

//=============================================================================
//! \brief State variables what we monitor from the vehicle. Notation: $X$
//! \param p_position position [m]. Notation: $p$
//! \param p_velocity velocity [m/s]. Notation: $v$
//! \param p_acceleration acceleration [m/s²]. Notation: $a$
//!
//! X = [position; velocity; acceleration] = [p; v; a]
//! k is current time step.
//! X_k is the state vector at time step k: [p_k; v_k; a_k]
//!
//! State transition equation:
//! X_{k+1} = A X_k + B U_k
//!
//! where:
//! A is 3x3 matrix:
//! A = [1   dt  0.5*dt^2
//!      0    1      dt
//!      0  -b/m     0]
//!
//! B is 3x1 matrix:
//! B = [0; 0; 1/m]
//!
//! U is 1x1 matrix:
//! U = [u_k]
//!
//! where:
//! u_k is the vehicle control input [N] at time step k.
//=============================================================================
struct VehicleState
{
    VehicleState(double p_position = 0.0, double p_speed = 0.0, double p_acceleration = 0.0)
        : position(p_position), velocity(p_speed), acceleration(p_acceleration)
    {}

    double position;    // [m]
    double velocity;    // [m/s]
    double acceleration;// [m/s²]
};

//=============================================================================
//! \brief Reference states: where we want to go. Notation: $X_{ref}$
//! \param p_position position [m]. Notation: $p_{ref}$
//! \param p_speed velocity [m/s]. Notation: $v_{ref}$
//! \param p_acceleration acceleration [m/s²]. Notation: $a_{ref}$
//!
//! X_{ref} = [p_{ref}; v_{ref}; a_{ref}]
//=============================================================================
struct Reference
{
    Reference(double p_position = 0.0, double p_speed = 0.0, double p_acceleration = 0.0)
        : position(p_position), velocity(p_speed), acceleration(p_acceleration)
    {}

    double position;    // [m]
    double velocity;    // [m/s]
    double acceleration;// [m/s²]
};

//=============================================================================
//! \brief Constraints on the state variables:
//! [p_min, p_max]: constrain on position: stay on the road.
//! [v_min, v_max]: constrain on velocity: road speed limit.
//! [a_min, a_max]: constrain on acceleration: physical confort.
//! [u_min, u_max]: constrain on control: engine limitation.
//=============================================================================
struct Constraints
{
    Constraints(double p_min_pos = 0.0, double p_max_pos = 0.0,
                double p_min_vel = 0.0, double p_max_vel = 0.0,
                double p_min_acc = 0.0, double p_max_acc = 0.0,
                double p_min_ctrl = 0.0, double p_max_ctrl = 0.0)
        : min_position(p_min_pos), max_position(p_max_pos),
          min_velocity(p_min_vel), max_velocity(p_max_vel),
          min_acceleration(p_min_acc), max_acceleration(p_max_acc),
          min_control(p_min_ctrl), max_control(p_max_ctrl)
    {}

    double min_position;    // [m]
    double max_position;    // [m]
    double min_velocity;    // [m/s]
    double max_velocity;    // [m/s]
    double min_acceleration;// [m/s²]
    double max_acceleration;// [m/s²]
    double min_control;    // [N]
    double max_control;    // [N]
};

//=============================================================================
//! \brief Parameters of the cost function.
//! Q: ponderation matrix for the state error (position, velocity, acceleration).
//! R: ponderation matrix for the control effort (engine force).
//=============================================================================
struct CostFunction
{
    CostFunction(double p_Q_pos = 0.0, double p_Q_vel = 0.0, double p_Q_acc = 0.0, double p_R_ctrl = 0.0)
        : Q_position(p_Q_pos), Q_velocity(p_Q_vel), Q_acceleration(p_Q_acc), R_control(p_R_ctrl)
    {}

    double Q_position;      // weight for the position error
    double Q_velocity;      // weight for the velocity error
    double Q_acceleration;  // weight for the acceleration error
    double R_control;       // weight for the control effort
};

//=============================================================================
//! \brief Class for the Model Predictive Control (MPC) controller of the vehicle.
//=============================================================================
class VehicleMPC
{
public:

    //-------------------------------------------------------------------------
    //! \brief Constructor of the MPC controller.
    //! \param p_dt time step of the simulation [s].
    //! \param p_prediction_horizon Number of seconds for the prediction [s].
    //! \param p_vehicle_params Model of the vehicle.
    //! \param p_cost_function Cost function of the MPC (Q, R).
    //! \param p_constraints Constraints to the state variables.
    //-------------------------------------------------------------------------
    VehicleMPC(double p_dt, double p_prediction_horizon, VehicleParams p_vehicle_params,
             CostFunction p_cost_function, Constraints p_constraints)
        : m_dt(p_dt), m_prediction_horizon(p_prediction_horizon / p_dt),
          m_vehicle_params(p_vehicle_params), m_cost_function(p_cost_function),
          m_constraints(p_constraints)
    {}

    //-------------------------------------------------------------------------
    //! \brief Prediction function for the dynamic model of the vehicle.
    //! \param p_current_state Current state of the vehicle.
    //! \param p_control Control input [N].
    //! \note Apply the state transition equation: X_{k+1} = A X_k + B u_k.
    //! \return Next state of the vehicle.
    //-------------------------------------------------------------------------
    VehicleState predictNextState(const VehicleState& p_current_state, double p_control)
    {
        VehicleState next_state;

        // Current acceleration based on the applied force and friction
        double current_acceleration = (p_control - m_vehicle_params.friction * p_current_state.velocity) / m_vehicle_params.mass;

        // Calculation of the new state according to the motion equations
        next_state.position = p_current_state.position + p_current_state.velocity * m_dt + 0.5 * current_acceleration * m_dt * m_dt;
        next_state.velocity = p_current_state.velocity + current_acceleration * m_dt;
        next_state.acceleration = current_acceleration;

        // Application of the constraints on the state
        constrain(next_state.position, m_constraints.min_position, m_constraints.max_position);
        constrain(next_state.velocity, m_constraints.min_velocity, m_constraints.max_velocity);
        constrain(next_state.acceleration, m_constraints.min_acceleration, m_constraints.max_acceleration);

        return next_state;
    }

    //-------------------------------------------------------------------------
    //! \brief Cost function: evaluate the quality of a control sequence.
    //! Apply the cost function:
    //! J = \sum_{i=0}^{N-1} (X_ref - X)' Q (X_ref - X)
    //!   + \sum_{i=0}^{N-1} u_i' R u_i.
    //! \param p_initial_state Initial state of the vehicle.
    //! \param p_control_seq Control sequence.
    //! \return Total cost of the control sequence.
    //-------------------------------------------------------------------------
    double calculateCost(const VehicleState& p_initial_state, const std::vector<double>& p_control_seq)
    {
        double total_cost = 0.0;
        VehicleState state = p_initial_state;

        // For each step in the prediction horizon
        for (size_t i = 0; (i < m_prediction_horizon) && (i < p_control_seq.size()); ++i)
        {
            double control = p_control_seq[i];

            // Cost of control effort
            // Compute: u_i' R u_i
            total_cost += m_cost_function.R_control * control * control;

            // Predict the next state X_{k+1} = A X_k + B u_k
            state = predictNextState(state, control);

            // Cost of deviation from the reference (desired state)
            // Compute: (X_ref - X)' Q (X_ref - X)
            total_cost += m_cost_function.Q_position * pow(state.position - m_reference.position, 2);
            total_cost += m_cost_function.Q_velocity * pow(state.velocity - m_reference.velocity, 2);
            total_cost += m_cost_function.Q_acceleration * pow(state.acceleration - m_reference.acceleration, 2);
        }

        // Return J
        return total_cost;
    }

    //-------------------------------------------------------------------------
    //! \brief Optimization algorithm to find the best control sequence.
    //! Uses a simple sampling method based on a binary search.
    //! \note better to use a more sophisticated method: quadratic programming.
    //! \param p_current_state Current state of the vehicle.
    //! \return Best control sequence.
    //-------------------------------------------------------------------------
    std::vector<double> optimizeControl(const VehicleState& p_current_state)
    {
        // Number of samples to test for the first command
        const size_t num_samples = 25;
        // Number of refinement iterations
        const size_t refinement_steps = 3;

        double best_first_control = 0.0;
        double min_cost = INFINITY;

        double control_range = m_constraints.max_control - m_constraints.min_control;
        double search_range = control_range;
        double search_center = (m_constraints.max_control + m_constraints.min_control) / 2.0;

        // Refinement iterations to find a better solution
        for (size_t step = 0; step < refinement_steps; ++step)
        {
            for (size_t i = 0; i < num_samples; ++i)
            {
                double first_control = search_center - search_range / 2.0 + search_range * double(i) / double(num_samples - 1);
                constrain(first_control, m_constraints.min_control, m_constraints.max_control);

                // Create a constant control sequence ()
                std::vector<double> control_sequence(m_prediction_horizon, first_control);

                // Calculate the cost for this sequence
                double cost = calculateCost(p_current_state, control_sequence);

                // Update if it's the best command found
                if (cost < min_cost)
                {
                    min_cost = cost;
                    best_first_control = first_control;
                }
            }

            // Reduce the search range around the best value found
            search_range = search_range / 2.0;
            search_center = best_first_control;
        }

        // Return the optimal sequence (with the first command optimized)
        std::vector<double> optimal_sequence(m_prediction_horizon, best_first_control);
        return optimal_sequence;
    }

    //-------------------------------------------------------------------------
    //! \brief Main function that computes the optimal control.
    //! \param p_current_state Current state of the vehicle.
    //! \return Optimal control sequence.
    //-------------------------------------------------------------------------
    double computeOptimalControl(const VehicleState& p_current_state)
    {
        std::vector<double> optimal_sequence = optimizeControl(p_current_state);
        return optimal_sequence[0];  // Only the first command is applied
    }

    //-------------------------------------------------------------------------
    //! \brief Accessor to modify the desired state.
    //! \param p_reference Desired state to set.
    //-------------------------------------------------------------------------
    void setReference(const Reference& p_reference) { m_reference = p_reference; }

private:

    //! \brief Time step [s]
    double m_dt;
    //! \brief Prediction horizon [steps]
    size_t m_prediction_horizon;
    //! \brief Parameters of the vehicle
    VehicleParams m_vehicle_params;
    //! \brief Parameters of the cost function
    CostFunction m_cost_function;
    //! \brief Constraints
    Constraints m_constraints;
    //! \brief Reference (target)
    Reference m_reference;
};

//=============================================================================
//! \brief Reference function that changes progressively.
//! The MPC will adapt to the new reference.
//! \param t Time [s].
//! \return Reference state.
//=============================================================================
Reference reference_function(double t)
{
    // Transition parameters
    double transition_start_time = 3.5;  // Start time of the transition [s]
    double transition_duration = 4.0;    // Duration of the transition [s]

    // Reference states
    Reference initial_ref(
        100.0, // Position [m]
        20.0,  // Velocity [m/s]
        0.0    // Acceleration [m/s²]
    );  
    Reference final_ref(
        200.0,  // Position [m]
        10.0,   // Velocity [m/s] 
        0.0     // Acceleration [m/s²]
    );

    // If before the transition, use the initial reference
    if (t < transition_start_time)
        return initial_ref;

    // If after the transition, use the final reference
    if (t >= transition_start_time + transition_duration)
        return final_ref;

    // Linear interpolation between the initial and final reference
    double alpha = (t - transition_start_time) / transition_duration;
    double pos = initial_ref.position * (1.0 - alpha) + final_ref.position * alpha;
    double vel = initial_ref.velocity * (1.0 - alpha) + final_ref.velocity * alpha;
    double acc = (final_ref.velocity - initial_ref.velocity) / transition_duration;
    return Reference(pos, vel, acc);
};

//=============================================================================
//! \brief Simple longitudinal control of a vehicle using MPC.
//=============================================================================
int main()
{
    // Simulation parameters
    double dt = 0.1;             // Time step [s]
    double sim_time = 12.0;      // Total simulation time [s]

    Constraints constraints(
        -1000.0, 1000.0, // Position limits [m]
        0.0, 32.0,       // Velocity limits [m/s]
        -5.0, 5.0,       // Acceleration limits [m/s²]
        -5000.0, 5000.0  // Control limits [N]
    );

    CostFunction cost_function(
        100.0,     // Importance of the position error
        10.0,    // Importance of the velocity error
        1.0,     // Importance of the acceleration error
        0.0001    // Importance of the control effort
    );

    VehicleParams vehicle_params(
        1500.0,    // Mass of the vehicle [kg]
        50.0       // Friction coefficient [-]
    );

    VehicleState initial_state(
        0.0,      // Position [m]
        15.0,     // Velocity [m/s]
        0.0       // Acceleration [m/s²]
    );

    // Reference states
    Reference target1(
        100.0,  // Position [m]
        20.0,   // Velocity [m/s]
        0.0     // Acceleration [m/s²]
    );
    Reference target2(
        150.0,  // Position [m]
        10.0,   // Velocity [m/s]
        0.0     // Acceleration [m/s²]
    );

    // Prediction horizon of MPC [s]
    size_t prediction_horizon = 3.0;

    // Creation of the MPC controller
    VehicleMPC controller(dt, prediction_horizon, vehicle_params, cost_function, constraints);

    // Current state (starts from the initial state)
    VehicleState current_state = initial_state;


    // Header for the results display
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Simulation of the MPC control for a vehicle" << std::endl;
    std::cout << "Time(s);RefPos(m);RefVel(m/s);RefAcc(m/s²);Pos(m);Speed(m/s);Acc(m/s²);Command(N)" << std::endl;

    // Simulation loop
    for (double current_time = 0; current_time < sim_time; current_time += dt)
    {
        // Set the reference
        Reference reference = reference_function(current_time);
        controller.setReference(reference);

        // Calculation of the optimal control
        double control = controller.computeOptimalControl(current_state);

        // Display of the state and the control
        std::cout << current_time << ";"
                  << reference.position << ";"
                  << reference.velocity << ";"
                  << reference.acceleration << ";"
                  << current_state.position << ";"
                  << current_state.velocity << ";"
                  << current_state.acceleration << ";"
                  << control << std::endl;

        // Application of the command and update of the state
        current_state = controller.predictNextState(current_state, control);
    }

    return 0;
}

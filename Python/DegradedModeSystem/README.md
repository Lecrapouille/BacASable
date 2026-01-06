# Robot Expert System - Degraded Mode Management

A hybrid expert system for managing a humanoid robot's degraded operational modes. This system uses **Python** for sensor state simulation and **SWI-Prolog** for logical inference, demonstrating how declarative programming can be integrated with imperative code.

## 🎯 Project Purpose

This project implements an expert system that:

1. **Simulates robot component states** (motors, sensors) using Python
2. **Infers available actions** based on component failures using Prolog's rule-based reasoning
3. **Determines system severity levels** (nominal, light degradation, severe degradation, critical)
4. **Provides explanations** for why certain actions are unavailable

The system demonstrates a practical application of **hybrid AI** where:

- **Python** handles the simulation layer (sensor states, component updates)
- **Prolog** handles the inference layer (rule-based reasoning, logical deduction)

## 📁 Project Structure

```
Failure/
├── degraded_mode_system.py    # Python interface and sensor simulation
├── degraded_mode_rules.pl     # Prolog knowledge base and inference rules
└── README.md                  # This file
```

### Files Description

- **`degraded_mode_system.py`**: Python module that:
  - Manages robot component states (arm motor, shoulder motor, elbow motor, Hall sensor)
  - Synchronizes states with Prolog
  - Queries Prolog for available actions and severity levels
  - Generates diagnostic reports for degraded modes

- **`degraded_mode_rules.pl`**: Prolog knowledge base containing:
  - Component state facts
  - Action availability rules (when can the robot perform each action)
  - Severity level determination rules (nominal, light/severe degradation, critical)
  - Unavailability reason explanations

## 🤖 Robot Components

The system monitors four key components:

1. **Arm Motor** (`arm_motor`) - Controls arm movement
2. **Shoulder Motor** (`shoulder_motor`) - Controls shoulder rotation
3. **Elbow Motor** (`elbow_motor`) - Controls elbow flexion
4. **Hand Hall Sensor** (`hand_hall_sensor`) - Detects object grasping

Each component can be in one of three states:
- `operational` - Component working normally
- `degraded` - Component partially functional
- `failed` - Component completely non-functional

## 🎮 Available Actions

The robot can perform six actions (when conditions are met):

1. **Wave Hand** (`wave_hand`) - Simple waving motion
2. **Grasp Object** (`grasp_object`) - Pick up objects
3. **Move Arm** (`move_arm`) - General arm movement
4. **Lift Object** (`lift_object`) - Lift and hold objects
5. **Point** (`point`) - Pointing gesture
6. **Full Manipulation** (`full_manipulation`) - Complete manipulation tasks

## 🔍 How It Works

### Architecture

```
┌──────────────────────┐
│   Python Layer       │  Simulates sensor states
│ (degraded_mode_sys)  │  Updates component states
└──────────┬───────────┘
           │ Queries Prolog
           │ (update_state, available_actions, severity_level)
           ▼
┌──────────────────────┐
│  Prolog Layer        │  Applies inference rules
│ (degraded_mode_rules)│  Determines available actions
└──────────────────────┘
```

### Workflow

1. **Python** updates a component state (e.g., `arm_motor` → `failed`)
2. **Python** synchronizes the state with Prolog via `update_state/2`
3. **Python** queries Prolog for available actions via `available_actions/1`
4. **Prolog** applies its rules to determine which actions are possible
5. **Python** receives the results and generates a diagnostic report

### Example Rule (Prolog)

```prolog
can_perform(grasp_object) :-
    operational(hand_hall_sensor),
    operational(arm_motor),
    operational(shoulder_motor),
    operational(elbow_motor).
```

This rule states: "The robot can grasp objects only if all four components are operational."

## 🚀 Installation

### Prerequisites

1. **Python 3.7+**
2. **SWI-Prolog** (version 9.0+)
3. **pyswip** Python package

### Setup

1. **Install SWI-Prolog**:
   ```bash
   # Fedora/RHEL
   sudo dnf install swi-prolog

   # Ubuntu/Debian
   sudo apt install swi-prolog

   # macOS
   brew install swi-prolog
   ```

2. **Install Python dependencies**:
   ```bash
   pip install pyswip
   ```

## 📖 Usage

### Basic Usage

```python
from degraded_mode_system import RobotExpertSystem, Component, ComponentState

# Initialize the degraded mode management system
robot = RobotExpertSystem()

# Simulate a component failure
robot.update_component_state(Component.HAND_HALL_SENSOR, ComponentState.FAILED)

# Get diagnosis
print(robot.diagnose())

# Check available actions
result = robot.infer_available_actions()
print(f"Available actions: {result['actions']}")
print(f"Severity: {result['severity']}")
```

### Running Test Scenarios

```bash
python degraded_mode_system.py
```

This will run 5 test scenarios:
1. Nominal operation (all components working)
2. Hall sensor failure
3. Arm motor failure
4. Shoulder motor failure
5. Multiple failures (critical)

### Direct Prolog Usage

You can also use Prolog directly:

```prolog
?- consult('degraded_mode_rules.pl').
?- update_state(arm_motor, failed).
?- available_actions(Actions).
?- severity_level(Severity).
?- can_perform(grasp_object).
```

## 🧪 Example Output

```
============================================================
HUMANOID ROBOT DIAGNOSTIC (Prolog inference)
============================================================

Component states (simulated by Python):
  ✓ arm_motor           : operational
  ✗ hand_hall_sensor    : failed
  ✓ shoulder_motor      : operational
  ✓ elbow_motor         : operational

Inference: Prolog Inference
Failure count: 1
Severity level: LIGHT_DEGRADATION
Reason: Inferred by Prolog

Available actions (3) [determined by Prolog]:
  • move_arm
  • point
  • wave_hand

Unavailable actions:
  ✗ grasp_object: Hall effect sensor failed
  ✗ lift_object: Grasping system failed
============================================================
```

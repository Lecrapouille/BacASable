from enum import Enum
from typing import Dict, Optional
from pathlib import Path

try:
    from pyswip import Prolog
except ImportError:
    raise ImportError("pyswip not available. Install it with: pip install pyswip")

class ComponentState(Enum):
    """Possible states for robot components"""
    OPERATIONAL = "operational"
    DEGRADED = "degraded"
    FAILED = "failed"

class Component(Enum):
    """Robot hardware components"""
    ARM_MOTOR = "arm_motor"
    HAND_HALL_SENSOR = "hand_hall_sensor"
    SHOULDER_MOTOR = "shoulder_motor"
    ELBOW_MOTOR = "elbow_motor"

class Action(Enum):
    """Possible robot actions"""
    WAVE_HAND = "wave_hand"
    GRASP_OBJECT = "grasp_object"
    MOVE_ARM = "move_arm"
    LIFT_OBJECT = "lift_object"
    POINT = "point"
    FULL_MANIPULATION = "full_manipulation"

class RobotExpertSystem:
    """Expert system: Python simulates sensor states, Prolog handles inference."""

    def __init__(self, prolog_file: Optional[str] = None):
        self.prolog = Prolog()
        prolog_file = prolog_file or Path(__file__).parent / "degraded_mode_rules.pl"
        self.prolog.consult(str(prolog_file).replace("\\", "/"))
        print(f"✓ Prolog file loaded: {prolog_file}")

        self.component_states = {c: ComponentState.OPERATIONAL for c in Component}
        self.reset()

    def reset(self):
        """Reset all components to operational state"""
        list(self.prolog.query("reset"))
        self.component_states = {c: ComponentState.OPERATIONAL for c in Component}
        print("✓ System reset")

    def update_component_state(self, component: Component, state: ComponentState):
        """Update a component's state in Python and Prolog"""
        self.component_states[component] = state
        list(self.prolog.query(f"update_state({component.value}, {state.value})"))
        print(f"[UPDATE] {component.value}: {state.value}")

    def infer_available_actions(self) -> Dict:
        """Infer available actions by querying Prolog"""
        # Get available actions
        results = list(self.prolog.query("available_actions(Actions)"))
        available_actions = set()
        if results:
            for action_name in results[0].get("Actions", []):
                try:
                    available_actions.add(Action(str(action_name)))
                except ValueError:
                    pass

        # Get severity level
        severity_results = list(self.prolog.query("severity_level(Severity)"))
        severity = str(severity_results[0]["Severity"]) if severity_results else "unknown"

        # Get reason if no actions available
        reason = "Inferred by Prolog"
        if not available_actions:
            for action in Action:
                reason_results = list(self.prolog.query(f"unavailability_reason({action.value}, Reason)"))
                if reason_results:
                    reason = str(reason_results[0]["Reason"])
                    break

        return {"actions": available_actions, "severity": severity, "reason": reason}

    def diagnose(self) -> str:
        """Generate complete diagnosis using Prolog inference"""
        result = self.infer_available_actions()
        failure_results = list(self.prolog.query("failure_count(N)"))
        failure_count = int(failure_results[0]["N"]) if failure_results else 0

        lines = [
            "=" * 60,
            "HUMANOID ROBOT DIAGNOSTIC (Prolog inference)",
            "=" * 60,
            "",
            "Component states (simulated by Python):"
        ]

        for comp, state in self.component_states.items():
            symbol = "✓" if state == ComponentState.OPERATIONAL else "✗"
            lines.append(f"  {symbol} {comp.value:20s}: {state.value}")

        lines.extend([
            "",
            f"Inference: Prolog Inference",
            f"Failure count: {failure_count}",
            f"Severity level: {result['severity'].upper()}",
            f"Reason: {result['reason']}",
            "",
            f"Available actions ({len(result['actions'])}) [determined by Prolog]:"
        ])

        if result['actions']:
            lines.extend(f"  • {a.value}" for a in sorted(result['actions'], key=lambda x: x.value))
        else:
            lines.append("  ⚠ NO ACTION AVAILABLE")

        lines.append("\nUnavailable actions:")
        for action in Action:
            if action not in result['actions']:
                reason_results = list(self.prolog.query(f"unavailability_reason({action.value}, Reason)"))
                if reason_results:
                    lines.append(f"  ✗ {action.value}: {reason_results[0]['Reason']}")

        lines.append("=" * 60)
        return "\n".join(lines)


# Demonstration main program
if __name__ == "__main__":
    robot = RobotExpertSystem()

    # Scenario 1: Everything works normally
    print("\n🤖 SCENARIO 1: Nominal operation")
    print(robot.diagnose())

    # Scenario 2: Hall sensor failure
    print("\n🤖 SCENARIO 2: Hall effect sensor failure")
    robot.update_component_state(Component.HAND_HALL_SENSOR, ComponentState.FAILED)
    print(robot.diagnose())

    # Scenario 3: Arm motor failure
    print("\n🤖 SCENARIO 3: Arm motor failure")
    robot.reset()
    robot.update_component_state(Component.ARM_MOTOR, ComponentState.FAILED)
    print(robot.diagnose())

    # Scenario 4: Shoulder motor failure
    print("\n🤖 SCENARIO 4: Shoulder motor failure")
    robot.reset()
    robot.update_component_state(Component.SHOULDER_MOTOR, ComponentState.FAILED)
    print(robot.diagnose())

    # Scenario 5: Multiple failures
    print("\n🤖 SCENARIO 5: Multiple failures (critical)")
    robot.reset()
    robot.update_component_state(Component.SHOULDER_MOTOR, ComponentState.FAILED)
    robot.update_component_state(Component.HAND_HALL_SENSOR, ComponentState.FAILED)
    print(robot.diagnose())
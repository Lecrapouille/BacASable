# Orbital Camera

Orbital Camera. This code is a port to Godot from the Unity3D code made by the
YouTube channel Game Dev Guide "Building a Camera Controller for a Strategy Game"
https://youtu.be/rnqF6S7PfFA

## Bindings

- `W` or `UP`: forward translation the camera.
- `S` or `DOWN`: backward translation the camera.
- `D` or `RIGHT`: right translation the camera.
- `A` or `LEFT`: left translation the camera.
- `R`: zoom in.
- `F`: zoom out.
- `O`: camera CCW rotation.
- `I`: camera CW rotation.
- left mouse button pressed: camera translation.
- middle mouse button pressed: camera rotation.
- mouse scroll: zoom (not functional).
- double left button click on the moving cube: track the cube.
- `ESCAPE`: stop tracking the cube.

## Issues

- Still buggy compared to original code.
- Quaternion lerp is not smooth.
- lerp using time is a poor idea since lerp expect a ratio between 0 .. 1.

## Scene Graph

- CameraRig: dummy node tracking a Camera. Used for offsetting the camera position. It holds the script for controlling the camera.
- MovingClickableCube: Uses sub-nodes Area + CollisionShape to allow mouse clicks (like Unity's onMouseDown()). Holds a script to make it move. Uses a timer to change the movement. Since the important
part is the camera control, this script is very basic.
- Cube1 ... Cube3: just dummy objects to simulate a real nicely modeled world.

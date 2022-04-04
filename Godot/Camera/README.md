# Orbital Camera

Oribtal Camera. This code is a portage to Godot code from the Unity3D code made by the
Youtube channel Game Dev Guide "Building a Camera Controller for a Strategy Game"
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
- mouse scroll: zoom (not fonctional).
- double left button click on the moving cube: track the cube.
- `ESCAPE`: stop tracking the cube.

## Issues
- Quaternion lerp is not smooth.
- lerp using time is a poor idea since lerp expect a ratio between 0 .. 1.

## Scene Graph
- CameraRig: dummy node tracking a Camera. Used for offseting the camera position. It holds the script for controlling the camera.
- MovingClickableCube: Use sub-nodes Area + CollisionShape to allow mouse click (like Unity's onMouseDown()). Hold a script to make move. Use a timer to change to movement. Since the important
part is the camera control, this script is verybasic.
- Cube1 ... Cube3: just dummy object to simulate a real nicely modelized world.

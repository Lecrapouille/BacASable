# This code is a portage to Godot code from the Unity3D code made by the
# Youtube channel Game Dev Guide. Please watch this video:
# https://youtu.be/rnqF6S7PfFA

extends Spatial

export(float) var movementTime = 5.0
export(float) var normalSpeed = 0.1
export(float) var fastSpeed = 1.0
onready var movementSpeed: float = normalSpeed
export(float) var rotationAmount = 3.0
export(Vector3) var zoomAmount = Vector3(0.0, -0.1, -0.1)

var tracking: Spatial = null
var tracking_offset: Vector3 = Vector3.ZERO
var newPosition: Vector3
var newRotation: Quat
var newZoom: Vector3

var dragStartPosition: Vector3
var dragCurrentPosition: Vector3
var rotateStartPosition: Vector2
var rotateCurrentPosition: Vector2

var q: Quat
var left_prev_pressed: bool = false
var left_pressed: bool = false
var right_prev_pressed: bool = false
var right_pressed: bool = false

# Fake world plane
var plane = Plane(Vector3.UP, 0.0)

func _ready():
	newPosition = self.transform.origin
	newRotation = Quat(self.transform.basis)
	newZoom = $Camera.transform.origin
	pass

func _input(event):
	# Change the camera displacement velocity
	if event is InputEventKey:
		if event.shift:
			movementSpeed = fastSpeed
		else:
			movementSpeed = normalSpeed
	pass

func handle_mouse_input(dt):
	# Mouse "scrolled" event: modify the camera zoom
	#if Input.is_mouse_button_pressed(BUTTON_WHEEL):
	#	newZoom += delta.y * zoomAmount

	# Mouse left button "on pressed" and "pressed" event: translate the camera.
	# Get the intersection point between the ray "camera ==> mouse cursor"
	# and the world plane. Rotate the camera of the difference of position
	# between the initial intersection position when the mouse left button
	# was pressed and the current intersection position.
	left_prev_pressed = left_pressed
	right_prev_pressed = right_pressed
	left_pressed = Input.is_mouse_button_pressed(BUTTON_LEFT)
	if left_pressed:
		var camera = $Camera
		# Get the current mouse position in the screen (camera's viewport)
		var mouse_pos = get_viewport().get_mouse_position()
		# Origin point of the ray
		var start = camera.project_ray_origin(mouse_pos)
		# Destination point of the ray
		var end = start + camera.project_ray_normal(mouse_pos) * camera.get_zfar()
		# Ray hit the world plane ?
		var intersection = plane.intersects_ray(start, end)
		# Rotate the camera if intersection happens (always true !?)
		if intersection != null:
			if (not left_prev_pressed) and left_pressed:
				dragStartPosition = intersection
			dragCurrentPosition = intersection
			newPosition = self.transform.origin + dragStartPosition - dragCurrentPosition

	# Mouse right button "on pressed" and "pressed" event: rotate the camera.
	right_pressed = Input.is_mouse_button_pressed(BUTTON_MIDDLE)
	if right_pressed:
		if not right_prev_pressed:
			rotateStartPosition = get_viewport().get_mouse_position()
		rotateCurrentPosition = get_viewport().get_mouse_position()
		var diff = rotateStartPosition - rotateCurrentPosition
		rotateStartPosition = rotateCurrentPosition
		q.set_euler(Vector3.UP * diff.x / -5.0 * dt)
		newRotation *= q
	pass

func handle_movement_input(dt):
	# Translate the camera long one of X-Y-Z axis
	if Input.is_key_pressed(KEY_W) or Input.is_key_pressed(KEY_UP):
		newPosition += (transform.basis.z * -movementSpeed)
	if Input.is_key_pressed(KEY_S) or Input.is_key_pressed(KEY_DOWN):
		newPosition += (transform.basis.z * movementSpeed)
	if Input.is_key_pressed(KEY_D) or Input.is_key_pressed(KEY_RIGHT):
		newPosition += (transform.basis.x * movementSpeed)
	if Input.is_key_pressed(KEY_A) or Input.is_key_pressed(KEY_LEFT):
		newPosition += (transform.basis.x * -movementSpeed)
	# Camera zoom
	if Input.is_key_pressed(KEY_R):
		newZoom += zoomAmount
	if Input.is_key_pressed(KEY_F):
		newZoom -= zoomAmount
	# Camera rotation
	if Input.is_key_pressed(KEY_O):
		q.set_euler(Vector3.UP * rotationAmount)
		newRotation *= q
	if Input.is_key_pressed(KEY_I):
		q.set_euler(Vector3.UP * -rotationAmount)
		newRotation *= q
	# Make the camera displacement
	var w = 1.0 - exp(-movementTime * dt)
	transform.origin = lerp(transform.origin, newPosition, w)
	transform.basis = Basis(newRotation.slerp(Quat(transform.basis), w))
	$Camera.transform.origin = lerp($Camera.transform.origin, newZoom, w)
	pass

func track(object: Spatial):
	tracking = object
	tracking_offset = transform.origin - tracking.transform.origin
	pass

func _process(dt):
	if tracking != null:
		if Input.is_key_pressed(KEY_ESCAPE):
			# Untrack the object
			tracking = null
		else:
			# Keep tracking the object
			transform.origin = tracking.transform.origin + tracking_offset
	else:
		handle_mouse_input(dt)
		handle_movement_input(dt)
	pass

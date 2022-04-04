extends MeshInstance

const SPEED: float = 2.0
var directions = [ Vector3.RIGHT, -Vector3.RIGHT ]
var d: int = 0

func _ready():
	pass

# Switch the movement direction
func _on_Timer_timeout():
	d = (d + 1) % 2
	pass 

# Displace the cube
func _process(delta):
	transform.origin += directions[d] * SPEED * delta
	pass

# Has the user clicked on the object? Equivalent to unity onMouseDown()
# If yes, then make the camera tracks this moving object.
func _on_Area_input_event(camera, event, position, normal, shape_idx):
	if event is InputEventMouseButton:
		if event.button_index == BUTTON_LEFT and event.doubleclick:
			get_node("/root/Spatial/CameraRig").track(self)
	pass

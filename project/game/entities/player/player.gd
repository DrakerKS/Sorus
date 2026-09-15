# player.gd
extends CharacterBody3D

@export var move_speed : float = 5.0

func _physics_process(delta: float) -> void:
	var input_dir := Input.get_vector("ui_left","ui_right","ui_up","ui_down")

	velocity.x = input_dir.x * move_speed
	velocity.z = input_dir.y * move_speed

	if input_dir.length() > 0.0:
		#var target_rotation := Vector2(input_dir.x, input_dir.y).angle() - deg_to_rad(14)
		var direction := Vector3(input_dir.x, 0.0, input_dir.y).normalized()

		#rotation.y = lerp_angle(rotation.y, -target_rotation, delta * 10.0)
		rotation.y = lerp_angle(rotation.y, atan2(direction.x, direction.z), delta * 15.0)
		print(rotation.y)

	move_and_slide()

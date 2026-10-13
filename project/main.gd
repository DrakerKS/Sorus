# main.gd
extends Node3D

@export_custom(PROPERTY_HINT_TYPE_STRING, "BoxShape3D") var foo : String
@onready var player = $World/PlayerPivot/Player
@onready var camera = $World/CameraPivot

func _ready() -> void:
	Debug.set_verbose(false)
	pass

func _process(_delta: float) -> void:
	var cam_pos = camera.global_position

	cam_pos.x = player.global_position.x + 1
	cam_pos.z = player.global_position.z + 3

	camera.global_position = cam_pos

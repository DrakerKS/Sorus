class_name SettingsMenu
extends Control

@export var d_value1 = null
@export var d_value3 = null
@export var dpi : DynamicPropertyInfo

func _ready() -> void:
	Debug.set_verbose(false)
# step * x = val
# val / step = x

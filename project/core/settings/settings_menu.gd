@tool

class_name SettingsMenu
extends Control

@export var d_value1 = null
@export var d_value2 = null
@export var d_property_info : DynamicPropertyInfo

func _ready() -> void:
	Debug.set_verbose(false)

#func _process(_delta: float) -> void:
	#print(d_value1)
	#print(d_value2)

@tool

class_name SettingsMenu
extends Control

@export_custom(TYPE_STRING, "24/17:DynamicPropertyInfo") var foo : String 
@export var d_value1 = null
@export var d_property_info : DynamicPropertyInfo

func _ready() -> void:
	Debug.set_verbose(false)
	
#func _process(_delta: float) -> void:
	#print("res://aeifefe/ldejfle.png".begins_with("res://"))
	#print("res://aeifefe/ldejfle.png".ends_with(".png"))

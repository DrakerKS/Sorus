@tool
extends EditorScript

func _run():
	var source: AnimationPlayer = load("res://assets/characters/taliyah_hidden_meshes.tscn").instantiate().get_node("AnimationPlayer")
	var library := AnimationLibrary.new()

	for name in source.get_animation_list():
		library.add_animation(name, Animation.new())

	var player := AnimationPlayer.new()
	player.name = "AnimationPlayer"
	player.add_animation_library("", library)

	var scene := PackedScene.new()
	scene.pack(player)
	ResourceSaver.save(scene, "res://assets/characters/anims_blueprint.tscn")

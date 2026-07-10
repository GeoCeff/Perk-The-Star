extends Node

const RUN_MODE_META: StringName = &"run_mode"


func _ready() -> void:
	_bind_mode("btn_play", "campaign")
	_bind_mode("btn_endless", "endless")


func _bind_mode(button_name: String, mode: String) -> void:
	var button := get_parent().get_node_or_null("CenterContainer/menu_box/button_box/%s" % button_name) as BaseButton
	if button == null:
		return
	button.pressed.connect(_set_run_mode.bind(mode))


func _set_run_mode(mode: String) -> void:
	# ponytail: metadata is enough for run mode; add a native model only if save slots need it.
	GameState.set_meta(RUN_MODE_META, mode)

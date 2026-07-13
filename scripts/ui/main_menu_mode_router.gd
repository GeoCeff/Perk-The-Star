extends Node

const RUN_MODE_META: StringName = &"run_mode"
const GAME_SCENE_PATH := "res://scenes/game.tscn"
const TECH_TREE_OVERLAY_SCENE_PATH := "res://scenes/ui/tech_tree_overlay.tscn"
const MODE_INFO := {
	"campaign": {
		"title": "Normal Defense",
		"kicker": "12-WAVE PRIME CAMPAIGN",
		"body": "Classic defense run. Build a stable orbit, answer each enemy type, and survive the Prime finale.",
		"note": "Best for steady Tech XP and learning tower paths.",
	},
	"boss_rush": {
		"title": "Boss Rush",
		"kicker": "COMPRESSED PRIME PRESSURE",
		"body": "Four heavy waves built around Prime-class threats. Less ramp-up, more immediate crisis control.",
		"note": "Bio-Lab shell cracking matters early.",
	},
	"daily_seed": {
		"title": "Daily Seed",
		"kicker": "FIXED DAILY CHALLENGE",
		"body": "A six-wave formation locked to today's seed. Everyone gets the same enemy mix for cleaner replays.",
		"note": "Good for routing and score chasing.",
	},
	"draft_defense": {
		"title": "Draft Defense",
		"kicker": "CONTRACT BEFORE WAVE 1",
		"body": "Pick a package before the first wave, then play around its tower bonuses and restrictions.",
		"note": "Stronger identity, less autopilot.",
	},
	"endless": {
		"title": "Endless Defense",
		"kicker": "SCALING SURVIVAL",
		"body": "Enemy count, speed, and health keep climbing after the normal campaign limit.",
		"note": "Play for one more wave.",
	},
	"no_flare": {
		"title": "No-Flare Challenge",
		"kicker": "NO SOLAR FLARE SAFETY NET",
		"body": "Clear the Prime mission without Solar Flare. Your tower placement has to solve every breach.",
		"note": "Higher pressure, bonus Tech XP.",
	},
}

var SpaceTheme: RefCounted = ClassDB.instantiate("SpaceThemeNative") as RefCounted
var mode_overlay: Control
var mode_info_panel: PanelContainer
var mode_info_title: Label
var mode_info_kicker: Label
var mode_info_body: Label
var mode_info_note: Label


func _ready() -> void:
	var start_button := _menu_button("btn_play")
	if start_button != null:
		start_button.pressed.connect(_show_mode_overlay)
	var tech_button := _menu_button("btn_tech_tree")
	if tech_button != null:
		tech_button.pressed.connect(_show_tech_tree)
		if SpaceTheme != null:
			SpaceTheme.call("apply_secondary_button", tech_button, SpaceTheme.get("ICON_CODEX_PATH"))
			tech_button.add_theme_font_size_override("font_size", 20)


func _unhandled_input(event: InputEvent) -> void:
	if mode_overlay == null:
		return
	if event is InputEventKey:
		var key := event as InputEventKey
		if key.pressed and not key.echo and key.keycode == KEY_ESCAPE:
			_close_mode_overlay()
			get_viewport().set_input_as_handled()


func _menu_button(button_name: String) -> BaseButton:
	return get_parent().get_node_or_null("CenterContainer/menu_box/button_box/%s" % button_name) as BaseButton


func _show_mode_overlay() -> void:
	if mode_overlay != null:
		return
	mode_overlay = Control.new()
	mode_overlay.name = "ModeOverlay"
	mode_overlay.mouse_filter = Control.MOUSE_FILTER_STOP
	mode_overlay.set_anchors_preset(Control.PRESET_FULL_RECT)
	get_parent().add_child(mode_overlay)

	var shade := ColorRect.new()
	shade.name = "Shade"
	shade.color = Color(0.0, 0.0, 0.0, 0.62)
	shade.mouse_filter = Control.MOUSE_FILTER_STOP
	shade.set_anchors_preset(Control.PRESET_FULL_RECT)
	mode_overlay.add_child(shade)

	var panel := PanelContainer.new()
	panel.name = "ModePanel"
	panel.custom_minimum_size = Vector2(560, 570)
	panel.set_anchors_preset(Control.PRESET_CENTER)
	panel.offset_left = -280
	panel.offset_top = -285
	panel.offset_right = 280
	panel.offset_bottom = 285
	mode_overlay.add_child(panel)
	_build_mode_info_panel()

	var box := VBoxContainer.new()
	box.name = "ModeBox"
	box.alignment = BoxContainer.ALIGNMENT_CENTER
	box.add_theme_constant_override("separation", 12)
	panel.add_child(box)

	var title := Label.new()
	title.text = "SELECT DEFENSE MODE"
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	title.add_theme_font_size_override("font_size", 26)
	box.add_child(title)

	var subtitle := Label.new()
	subtitle.text = "Choose the run type before launch."
	subtitle.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	subtitle.add_theme_font_size_override("font_size", 14)
	subtitle.add_theme_color_override("font_color", Color(0.76, 0.88, 0.96, 0.92))
	box.add_child(subtitle)

	_add_mode_button(box, "Normal Defense", "campaign", true)
	_add_mode_button(box, "Boss Rush", "boss_rush", false)
	_add_mode_button(box, "Daily Seed", "daily_seed", false)
	_add_mode_button(box, "Draft Defense", "draft_defense", false)
	_add_mode_button(box, "Endless Defense", "endless", false)
	_add_mode_button(box, "No-Flare Challenge", "no_flare", false)

	var back := Button.new()
	back.text = "BACK"
	back.custom_minimum_size = Vector2(360, 48)
	back.pressed.connect(_close_mode_overlay)
	box.add_child(back)

	if SpaceTheme != null:
		SpaceTheme.call("apply_fonts", mode_overlay)
		SpaceTheme.call("apply_deep_panel", panel, SpaceTheme.get("COLOR_CYAN"))
		SpaceTheme.call("apply_deep_panel", mode_info_panel, SpaceTheme.get("COLOR_GOLD"))
		SpaceTheme.call("apply_secondary_button", back)
		title.add_theme_color_override("font_color", SpaceTheme.get("COLOR_GOLD"))
	var first_button := box.get_child(2) as Button
	if first_button != null:
		first_button.grab_focus()


func _add_mode_button(box: VBoxContainer, text: String, mode: String, primary: bool) -> void:
	var button := Button.new()
	button.text = text
	button.custom_minimum_size = Vector2(360, 52)
	button.pressed.connect(_start_mode.bind(mode))
	button.mouse_entered.connect(_show_mode_info.bind(mode))
	button.focus_entered.connect(_show_mode_info.bind(mode))
	box.add_child(button)
	if SpaceTheme != null:
		if primary:
			SpaceTheme.call("apply_primary_button", button, SpaceTheme.get("ICON_PLAY_PATH"))
		else:
			SpaceTheme.call("apply_secondary_button", button, SpaceTheme.get("ICON_PLAY_PATH"))


func _build_mode_info_panel() -> void:
	mode_info_panel = PanelContainer.new()
	mode_info_panel.name = "ModeInfoPanel"
	mode_info_panel.mouse_filter = Control.MOUSE_FILTER_IGNORE
	mode_info_panel.custom_minimum_size = Vector2(350, 0)
	mode_info_panel.set_anchors_preset(Control.PRESET_CENTER)
	mode_info_panel.offset_left = 310
	mode_info_panel.offset_top = -215
	mode_info_panel.offset_right = 660
	mode_info_panel.offset_bottom = 80
	mode_info_panel.visible = false
	mode_overlay.add_child(mode_info_panel)

	var box := VBoxContainer.new()
	box.name = "ModeInfoBox"
	box.add_theme_constant_override("separation", 8)
	mode_info_panel.add_child(box)

	mode_info_kicker = Label.new()
	mode_info_kicker.add_theme_font_size_override("font_size", 11)
	mode_info_kicker.add_theme_color_override("font_color", Color(0.22, 0.84, 0.94, 0.92))
	box.add_child(mode_info_kicker)

	mode_info_title = Label.new()
	mode_info_title.add_theme_font_size_override("font_size", 21)
	mode_info_title.add_theme_color_override("font_color", Color(1.0, 0.78, 0.26, 0.96))
	box.add_child(mode_info_title)

	mode_info_body = Label.new()
	mode_info_body.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	mode_info_body.add_theme_font_size_override("font_size", 14)
	mode_info_body.add_theme_color_override("font_color", Color(0.88, 0.94, 1.0, 0.96))
	box.add_child(mode_info_body)

	mode_info_note = Label.new()
	mode_info_note.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	mode_info_note.add_theme_font_size_override("font_size", 12)
	mode_info_note.add_theme_color_override("font_color", Color(0.62, 0.74, 0.86, 0.92))
	box.add_child(mode_info_note)


func _show_mode_info(mode: String) -> void:
	if mode_info_panel == null:
		return
	var info: Dictionary = MODE_INFO.get(mode, {})
	if info.is_empty():
		mode_info_panel.visible = false
		return
	mode_info_kicker.text = str(info.get("kicker", "")).to_upper()
	mode_info_title.text = str(info.get("title", "")).to_upper()
	mode_info_body.text = str(info.get("body", ""))
	mode_info_note.text = str(info.get("note", ""))
	mode_info_panel.visible = true


func _close_mode_overlay() -> void:
	if mode_overlay != null:
		mode_overlay.queue_free()
	mode_overlay = null
	mode_info_panel = null


func _show_tech_tree() -> void:
	if get_parent().get_node_or_null("TechTreeOverlay") != null:
		return
	var packed := load(TECH_TREE_OVERLAY_SCENE_PATH) as PackedScene
	if packed == null:
		push_error("Main menu could not load tech tree overlay.")
		return
	get_parent().add_child(packed.instantiate())


func _start_mode(mode: String) -> void:
	# ponytail: metadata is enough for run mode; add a native model only if save slots need it.
	GameState.set_meta(RUN_MODE_META, mode)
	if GameState.has_method("clear_test_run"):
		GameState.call("clear_test_run")
	var music := get_parent().get_node_or_null("/root/MusicManager")
	if music != null:
		music.call("stop_music")
	get_tree().change_scene_to_file(GAME_SCENE_PATH)

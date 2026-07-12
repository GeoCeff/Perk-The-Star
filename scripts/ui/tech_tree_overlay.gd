extends CanvasLayer


class TechTreeBoard:
	extends Control

	var links: Array = []
	var node_centers: Dictionary = {}
	var node_states: Dictionary = {}

	func _draw() -> void:
		for link in links:
			if link.size() < 2:
				continue
			var from_id: String = str(link[0])
			var to_id: String = str(link[1])
			if not node_centers.has(from_id) or not node_centers.has(to_id):
				continue
			var state_a: String = str(node_states.get(from_id, "locked"))
			var state_b: String = str(node_states.get(to_id, "locked"))
			var color := Color(0.20, 0.84, 0.94, 0.46)
			if state_a == "locked" or state_b == "locked":
				color = Color(0.20, 0.32, 0.42, 0.48)
			elif state_a == "master" or state_b == "master":
				color = Color(1.0, 0.78, 0.24, 0.62)
			var from_pos: Vector2 = node_centers[from_id]
			var to_pos: Vector2 = node_centers[to_id]
			draw_line(from_pos, to_pos, Color(0.0, 0.0, 0.0, 0.52), 8.0, true)
			draw_line(from_pos, to_pos, color, 4.0, true)
			draw_circle(from_pos, 5.0, color)
			draw_circle(to_pos, 5.0, color)


const UPGRADES: Array = [
	{
		"id": "solar_lens",
		"title": "Solar Lens",
		"row": 0,
		"col": 0,
		"tower": "photon_splitter",
		"cost": 75,
		"req": [],
		"desc": "Photon Splitters gain 10% range.",
	},
	{
		"id": "split_beam",
		"title": "Split Beam",
		"row": 0,
		"col": 1,
		"tower": "photon_splitter",
		"cost": 180,
		"req": ["solar_lens"],
		"desc": "Photon Splitters deal 12% more damage.",
	},
	{
		"id": "plasma_core",
		"title": "Plasma Core",
		"row": 0,
		"col": 2,
		"tower": "photon_splitter",
		"cost": 360,
		"req": ["split_beam"],
		"desc": "Photon Splitters fire 10% faster.",
	},
	{
		"id": "long_orbit",
		"title": "Long Orbit",
		"row": 1,
		"col": 0,
		"tower": "cryo_probe",
		"cost": 90,
		"req": [],
		"desc": "Cryo Probes gain 5% range.",
	},
	{
		"id": "far_sight",
		"title": "Far Sight",
		"row": 1,
		"col": 1,
		"tower": "cryo_probe",
		"cost": 240,
		"req": ["long_orbit"],
		"desc": "Cryo Probes gain 12% range and 25% longer slow.",
	},
	{
		"id": "brittle_shells",
		"title": "Brittle Shells",
		"row": 1,
		"col": 2,
		"tower": "cryo_probe",
		"cost": 470,
		"req": ["far_sight"],
		"desc": "Slowed enemies take 12% more non-control damage.",
	},
	{
		"id": "bio_splice",
		"title": "Bio Splice",
		"row": 2,
		"col": 0,
		"tower": "bio_lab",
		"cost": 220,
		"req": [],
		"desc": "Bio-Labs fire and excavate 12% faster.",
	},
	{
		"id": "salvage_culture",
		"title": "Salvage Culture",
		"row": 2,
		"col": 1,
		"tower": "bio_lab",
		"cost": 410,
		"req": ["bio_splice"],
		"desc": "Enemy kills pay 10% more Sol Credits.",
	},
	{
		"id": "solar_choir",
		"title": "Solar Choir",
		"row": 2,
		"col": 2,
		"tower": "bio_lab",
		"cost": 1350,
		"req": ["salvage_culture"],
		"desc": "Bio-Labs gain 10% damage and range.",
	},
	{
		"id": "rapid_charge",
		"title": "Rapid Charge",
		"row": 3,
		"col": 0,
		"tower": "magnetic_net",
		"cost": 110,
		"req": [],
		"desc": "Magnetic Nets pulse 5% faster.",
	},
	{
		"id": "magnetic_lattice",
		"title": "Magnetic Lattice",
		"row": 3,
		"col": 1,
		"tower": "magnetic_net",
		"cost": 280,
		"req": ["rapid_charge"],
		"desc": "Magnetic Nets gain 10% range.",
	},
	{
		"id": "gravitic_payload",
		"title": "Gravitic Payload",
		"row": 3,
		"col": 2,
		"tower": "magnetic_net",
		"cost": 840,
		"req": ["magnetic_lattice"],
		"desc": "Magnetic slows last 25% longer and Magnetic Nets gain 10% range.",
	},
	{
		"id": "stellar_lance",
		"title": "Stellar Lance",
		"row": 4,
		"col": 0,
		"tower": "helios_cannon",
		"cost": 620,
		"req": [],
		"desc": "Helios Cannons deal 14% more damage.",
	},
	{
		"id": "slingshot_coils",
		"title": "Slingshot Coils",
		"row": 4,
		"col": 1,
		"tower": "helios_cannon",
		"cost": 950,
		"req": ["stellar_lance"],
		"desc": "Helios Slingshot costs 30 Sol and hits 20% harder.",
	},
	{
		"id": "flare_battery",
		"title": "Flare Battery",
		"row": 4,
		"col": 2,
		"tower": "helios_cannon",
		"cost": 760,
		"req": ["slingshot_coils"],
		"desc": "Solar Flare charges every 2 cleared waves instead of 3.",
	},
	{
		"id": "pressure_hull",
		"title": "Pressure Hull",
		"row": 5,
		"col": 0,
		"tower": "tardigrade_bomb",
		"cost": 90,
		"req": [],
		"desc": "Tardigrade Bombs gain 10% range.",
	},
	{
		"id": "spore_nests",
		"title": "Spore Nests",
		"row": 5,
		"col": 1,
		"tower": "tardigrade_bomb",
		"cost": 240,
		"req": ["pressure_hull"],
		"desc": "Tardigrade Bombs fire 8% faster.",
	},
	{
		"id": "resilient_bloom",
		"title": "Resilient Bloom",
		"row": 5,
		"col": 2,
		"tower": "tardigrade_bomb",
		"cost": 470,
		"req": ["spore_nests"],
		"desc": "Tardigrade Bombs deal 14% more damage.",
	},
	{
		"id": "photon_apex",
		"title": "Apex Photon",
		"row": 0,
		"col": 3,
		"tower": "photon_splitter",
		"cost": 900,
		"req": ["plasma_core"],
		"desc": "Photon Splitters gain 8% damage, rate, and range.",
	},
	{
		"id": "cryo_apex",
		"title": "Apex Cryo",
		"row": 1,
		"col": 3,
		"tower": "cryo_probe",
		"cost": 1000,
		"req": ["brittle_shells"],
		"desc": "Cryo Probes gain 8% damage, rate, and range. Cryo slows last longer.",
	},
	{
		"id": "bio_apex",
		"title": "Apex Bio-Lab",
		"row": 2,
		"col": 3,
		"tower": "bio_lab",
		"cost": 1500,
		"req": ["solar_choir"],
		"desc": "Bio-Labs gain 8% damage, rate, and range.",
	},
	{
		"id": "magnetic_apex",
		"title": "Apex Magnetic",
		"row": 3,
		"col": 3,
		"tower": "magnetic_net",
		"cost": 1200,
		"req": ["gravitic_payload"],
		"desc": "Magnetic Nets gain 8% damage, rate, and range. Magnetic slows last longer.",
	},
	{
		"id": "helios_apex",
		"title": "Apex Helios",
		"row": 4,
		"col": 3,
		"tower": "helios_cannon",
		"cost": 1600,
		"req": ["flare_battery"],
		"desc": "Helios Cannons gain 8% damage, rate, and range. Solar Flare deals +10 damage.",
	},
	{
		"id": "tardigrade_apex",
		"title": "Apex Tardigrade",
		"row": 5,
		"col": 3,
		"tower": "tardigrade_bomb",
		"cost": 1000,
		"req": ["resilient_bloom"],
		"desc": "Tardigrade Bombs gain 8% damage, rate, and range.",
	},
	{
		"id": "apex_master",
		"title": "Apex Master",
		"row": 2.5,
		"col": 4,
		"tower": "bio_lab",
		"cost": 3000,
		"req": ["photon_apex", "cryo_apex", "bio_apex", "magnetic_apex", "helios_apex", "tardigrade_apex"],
		"desc": "All towers count as apex-tuned. Control slows last longer and Solar Flare hits harder.",
	},
]

const LINKS: Array = [
	["solar_lens", "split_beam"],
	["split_beam", "plasma_core"],
	["plasma_core", "photon_apex"],
	["long_orbit", "far_sight"],
	["far_sight", "brittle_shells"],
	["brittle_shells", "cryo_apex"],
	["bio_splice", "salvage_culture"],
	["salvage_culture", "solar_choir"],
	["solar_choir", "bio_apex"],
	["rapid_charge", "magnetic_lattice"],
	["magnetic_lattice", "gravitic_payload"],
	["gravitic_payload", "magnetic_apex"],
	["stellar_lance", "slingshot_coils"],
	["slingshot_coils", "flare_battery"],
	["flare_battery", "helios_apex"],
	["pressure_hull", "spore_nests"],
	["spore_nests", "resilient_bloom"],
	["resilient_bloom", "tardigrade_apex"],
	["photon_apex", "apex_master"],
	["cryo_apex", "apex_master"],
	["bio_apex", "apex_master"],
	["magnetic_apex", "apex_master"],
	["helios_apex", "apex_master"],
	["tardigrade_apex", "apex_master"],
]

const TOWER_ICONS: Dictionary = {
	"photon_splitter": "res://assets/sprites/clean/towers/photon_splitter.png",
	"cryo_probe": "res://assets/sprites/clean/towers/cryo_probe.png",
	"bio_lab": "res://assets/sprites/clean/towers/bio_lab.png",
	"magnetic_net": "res://assets/sprites/clean/towers/magnetic_net.png",
	"helios_cannon": "res://assets/sprites/clean/towers/helios_cannon.png",
	"tardigrade_bomb": "res://assets/sprites/clean/towers/tardigrade_bomb.png",
}

const TECH_TIER_ICON_PATH := "res://assets/ui/tech_tiers/%s_tier_%d.png"

const COLOR_DEEP := Color(0.006, 0.012, 0.024, 0.95)
const COLOR_CYAN := Color(0.22, 0.84, 0.94, 0.88)
const COLOR_GOLD := Color(1.0, 0.78, 0.26, 0.92)
const COLOR_MUTED := Color(0.58, 0.70, 0.82, 0.86)
const COLOR_TEXT := Color(0.90, 0.96, 1.0, 1.0)
const TREE_COLS := 5.0
const TREE_ROWS := 6.0

var SpaceTheme: RefCounted = ClassDB.instantiate("SpaceThemeNative") as RefCounted
var was_paused: bool = false
var selected_id: String = "solar_lens"
var upgrade_by_id: Dictionary = {}
var node_cards: Dictionary = {}

var root: Control
var shade: ColorRect
var close_button: Button
var effects_toggle_button: Button
var title_panel: PanelContainer
var main_panel: PanelContainer
var selected_panel: Panel
var tree_board: TechTreeBoard
var selected_box: VBoxContainer
var selected_title: Label
var selected_icon: TextureRect
var selected_status: Label
var selected_cost: Label
var selected_requires: Label
var selected_desc: Label
var unlock_button: Button
var title_label: Label
var xp_label: Label
var subtitle_label: Label


func _ready() -> void:
	process_mode = Node.PROCESS_MODE_ALWAYS
	was_paused = get_tree().paused
	get_tree().paused = true
	for upgrade in UPGRADES:
		upgrade_by_id[str(upgrade["id"])] = upgrade
	_build_overlay()
	_select_upgrade(selected_id)
	_refresh_tree()
	_layout_overlay()
	if get_viewport() != null and not get_viewport().size_changed.is_connected(_layout_overlay):
		get_viewport().size_changed.connect(_layout_overlay)


func _exit_tree() -> void:
	get_tree().paused = was_paused


func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventKey:
		var key_event := event as InputEventKey
		if key_event.pressed and not key_event.echo and (key_event.keycode == KEY_ESCAPE or key_event.keycode == KEY_T):
			_close()
			get_viewport().set_input_as_handled()


func _build_overlay() -> void:
	root = Control.new()
	root.name = "OverlayRoot"
	root.mouse_filter = Control.MOUSE_FILTER_STOP
	root.set_anchors_preset(Control.PRESET_FULL_RECT)
	add_child(root)

	shade = ColorRect.new()
	shade.name = "Shade"
	shade.color = Color(0.004, 0.009, 0.020, 0.78)
	shade.mouse_filter = Control.MOUSE_FILTER_STOP
	shade.set_anchors_preset(Control.PRESET_FULL_RECT)
	root.add_child(shade)

	main_panel = PanelContainer.new()
	main_panel.name = "TechPanel"
	main_panel.mouse_filter = Control.MOUSE_FILTER_STOP
	main_panel.add_theme_stylebox_override("panel", _panel_style(COLOR_DEEP, COLOR_CYAN, 8.0, 20.0, 16.0))
	root.add_child(main_panel)

	title_panel = PanelContainer.new()
	title_panel.name = "TitlePanel"
	title_panel.mouse_filter = Control.MOUSE_FILTER_IGNORE
	title_panel.add_theme_stylebox_override("panel", _panel_style(Color(0.026, 0.060, 0.110, 0.98), COLOR_CYAN, 8.0, 22.0, 14.0))
	root.add_child(title_panel)

	var title_box := VBoxContainer.new()
	title_box.name = "TitleBox"
	title_box.alignment = BoxContainer.ALIGNMENT_CENTER
	title_box.add_theme_constant_override("separation", 5)
	title_panel.add_child(title_box)

	var title_row := HBoxContainer.new()
	title_row.alignment = BoxContainer.ALIGNMENT_CENTER
	title_row.add_theme_constant_override("separation", 16)
	title_box.add_child(title_row)

	title_label = Label.new()
	title_label.name = "TitleLabel"
	title_label.text = "SOL TECH TREE"
	title_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	title_label.add_theme_font_size_override("font_size", 31)
	title_row.add_child(title_label)

	xp_label = Label.new()
	xp_label.name = "XpLabel"
	_update_xp_label()
	xp_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	xp_label.add_theme_font_size_override("font_size", 24)
	xp_label.add_theme_color_override("font_color", Color(0.50, 1.0, 0.28, 1.0))
	title_row.add_child(xp_label)

	subtitle_label = Label.new()
	subtitle_label.name = "SubtitleLabel"
	subtitle_label.text = "Each tower has its own path and apex version."
	subtitle_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	subtitle_label.add_theme_font_size_override("font_size", 15)
	subtitle_label.add_theme_color_override("font_color", Color(0.82, 0.90, 1.0, 0.94))
	title_box.add_child(subtitle_label)

	close_button = Button.new()
	close_button.name = "BackButton"
	close_button.text = "BACK"
	close_button.custom_minimum_size = Vector2(92, 58)
	close_button.pressed.connect(_close)
	root.add_child(close_button)

	effects_toggle_button = Button.new()
	effects_toggle_button.name = "TechEffectsToggleButton"
	effects_toggle_button.custom_minimum_size = Vector2(176, 58)
	effects_toggle_button.tooltip_text = "Toggle active tech upgrade effects."
	effects_toggle_button.pressed.connect(_toggle_tech_effects)
	root.add_child(effects_toggle_button)
	_refresh_effects_toggle_button()

	selected_panel = Panel.new()
	selected_panel.name = "SelectedTechPanel"
	selected_panel.mouse_filter = Control.MOUSE_FILTER_STOP
	selected_panel.add_theme_stylebox_override("panel", _panel_style(Color(0.018, 0.022, 0.044, 0.97), COLOR_GOLD, 8.0, 18.0, 14.0))
	root.add_child(selected_panel)
	_build_selected_panel()

	tree_board = TechTreeBoard.new()
	tree_board.name = "TechTreeBoard"
	tree_board.mouse_filter = Control.MOUSE_FILTER_PASS
	tree_board.links = LINKS
	root.add_child(tree_board)
	_build_upgrade_nodes()

	if SpaceTheme != null:
		SpaceTheme.call("apply_fonts", root)
		SpaceTheme.call("apply_secondary_button", close_button, SpaceTheme.get("ICON_BACK_PATH"))
		SpaceTheme.call("apply_secondary_button", effects_toggle_button)
		_apply_xp_label_font()
	close_button.add_theme_font_size_override("font_size", 15)
	effects_toggle_button.add_theme_font_size_override("font_size", 13)
	_refresh_effects_toggle_button()


func _build_selected_panel() -> void:
	selected_box = VBoxContainer.new()
	selected_box.name = "SelectedBox"
	selected_box.alignment = BoxContainer.ALIGNMENT_BEGIN
	selected_box.add_theme_constant_override("separation", 8)
	selected_panel.add_child(selected_box)

	selected_title = Label.new()
	selected_title.name = "SelectedTitle"
	selected_title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	selected_title.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	selected_title.add_theme_font_size_override("font_size", 19)
	selected_title.add_theme_color_override("font_color", COLOR_GOLD)
	selected_box.add_child(selected_title)

	selected_icon = TextureRect.new()
	selected_icon.name = "SelectedIcon"
	selected_icon.custom_minimum_size = Vector2(104, 104)
	selected_icon.expand_mode = TextureRect.EXPAND_FIT_WIDTH_PROPORTIONAL
	selected_icon.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	selected_box.add_child(selected_icon)

	selected_status = Label.new()
	selected_status.name = "SelectedStatus"
	selected_status.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	selected_status.add_theme_font_size_override("font_size", 17)
	selected_box.add_child(selected_status)

	selected_cost = Label.new()
	selected_cost.name = "SelectedCost"
	selected_cost.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	selected_cost.add_theme_font_size_override("font_size", 13)
	selected_box.add_child(selected_cost)

	selected_requires = Label.new()
	selected_requires.name = "SelectedRequires"
	selected_requires.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	selected_requires.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	selected_requires.add_theme_font_size_override("font_size", 11)
	selected_requires.add_theme_color_override("font_color", COLOR_MUTED)
	selected_box.add_child(selected_requires)

	selected_desc = Label.new()
	selected_desc.name = "SelectedDescription"
	selected_desc.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	selected_desc.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	selected_desc.add_theme_font_size_override("font_size", 14)
	selected_desc.add_theme_color_override("font_color", Color(0.82, 0.90, 1.0, 0.92))
	selected_box.add_child(selected_desc)

	unlock_button = Button.new()
	unlock_button.name = "UnlockButton"
	unlock_button.custom_minimum_size = Vector2(190, 48)
	unlock_button.pressed.connect(_try_unlock_selected)
	selected_box.add_child(unlock_button)
	if SpaceTheme != null:
		SpaceTheme.call("apply_primary_button", unlock_button)
	unlock_button.add_theme_font_size_override("font_size", 15)


func _build_upgrade_nodes() -> void:
	for upgrade in UPGRADES:
		var id := str(upgrade["id"])
		var state := _upgrade_state(upgrade)
		tree_board.node_states[id] = state

		var card := Control.new()
		card.name = "%sNode" % id.to_pascal_case()
		card.mouse_filter = Control.MOUSE_FILTER_PASS
		tree_board.add_child(card)
		node_cards[id] = card

		var button := Button.new()
		button.name = "IconButton"
		button.text = ""
		button.focus_mode = Control.FOCUS_ALL
		button.tooltip_text = _upgrade_tooltip(upgrade)
		button.expand_icon = true
		button.set_button_icon(_load_upgrade_icon(upgrade))
		button.pressed.connect(_select_upgrade.bind(id))
		card.add_child(button)
		_style_upgrade_button(button, state)

		var label := Label.new()
		label.name = "NodeLabel"
		label.text = str(upgrade["title"])
		label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
		label.add_theme_font_size_override("font_size", 13)
		label.add_theme_color_override("font_color", COLOR_TEXT if state != "locked" else COLOR_MUTED)
		card.add_child(label)

		var tag := Label.new()
		tag.name = "NodeTag"
		tag.text = _upgrade_tag(upgrade)
		tag.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		tag.add_theme_font_size_override("font_size", 9)
		tag.add_theme_color_override("font_color", _state_color(state))
		card.add_child(tag)


func _layout_overlay() -> void:
	if root == null:
		return
	var viewport_size: Vector2 = get_viewport().get_visible_rect().size

	var panel_size := Vector2(minf(viewport_size.x - 48.0, 1480.0), minf(viewport_size.y - 82.0, 840.0))
	panel_size.x = maxf(panel_size.x, 980.0)
	panel_size.y = maxf(panel_size.y, 610.0)
	var panel_pos := (viewport_size - panel_size) * 0.5 + Vector2(0.0, 18.0)
	panel_pos.x = maxf(20.0, panel_pos.x)
	panel_pos.y = maxf(18.0, panel_pos.y)
	main_panel.position = panel_pos
	main_panel.size = panel_size

	close_button.position = Vector2(panel_pos.x + 18.0, panel_pos.y + 16.0)
	close_button.size = Vector2(92.0, 58.0)
	effects_toggle_button.position = Vector2(panel_pos.x + panel_size.x - 196.0, panel_pos.y + 16.0)
	effects_toggle_button.size = Vector2(176.0, 58.0)

	var title_width: float = minf(panel_size.x - 460.0, 860.0)
	title_panel.position = Vector2(panel_pos.x + (panel_size.x - title_width) * 0.5, panel_pos.y + 14.0)
	title_panel.size = Vector2(title_width, 92.0)

	var content_top: float = panel_pos.y + 132.0
	var content_bottom: float = panel_pos.y + panel_size.y - 30.0
	var content_h: float = content_bottom - content_top
	var gap: float = 20.0
	var left_w: float = clampf(panel_size.x * 0.24, 260.0, 320.0)
	var tree_w: float = panel_size.x - left_w - gap * 3.0
	var x: float = panel_pos.x + gap

	selected_panel.position = Vector2(x, content_top)
	selected_panel.size = Vector2(left_w, content_h)
	if selected_box != null:
		selected_box.position = Vector2(18.0, 18.0)
		selected_box.size = selected_panel.size - Vector2(36.0, 36.0)
		selected_title.custom_minimum_size = Vector2(left_w - 48.0, 44.0)
		selected_desc.custom_minimum_size = Vector2(left_w - 48.0, 62.0)
		selected_requires.custom_minimum_size = Vector2(left_w - 48.0, 28.0)
	x += left_w + gap

	tree_board.position = Vector2(x, content_top + 18.0)
	tree_board.size = Vector2(tree_w, content_h - 36.0)

	_layout_upgrade_nodes()


func _layout_upgrade_nodes() -> void:
	if tree_board == null:
		return
	var board_size: Vector2 = tree_board.size
	var lane_h: float = board_size.y / TREE_ROWS
	var card_h: float = maxf(62.0, lane_h - 4.0)
	var node_size: float = minf(clampf(board_size.x / 13.0, 48.0, 76.0), card_h - 38.0)
	var card_size := Vector2(maxf(node_size + 34.0, 104.0), card_h)
	var centers: Dictionary = {}
	for upgrade in UPGRADES:
		var id := str(upgrade["id"])
		var row := float(upgrade["row"])
		var col := float(upgrade["col"])
		var center := Vector2(
			(board_size.x / TREE_COLS) * (col + 0.5),
			(board_size.y / TREE_ROWS) * (row + 0.50)
		)
		var card: Control = node_cards[id] as Control
		if card == null:
			continue
		card.position = center - Vector2(card_size.x * 0.5, card_size.y * 0.5)
		card.size = card_size
		var button := card.get_node("IconButton") as Button
		var label := card.get_node("NodeLabel") as Label
		var tag := card.get_node("NodeTag") as Label
		button.position = Vector2((card_size.x - node_size) * 0.5, 0.0)
		button.size = Vector2(node_size, node_size)
		label.position = Vector2(0.0, node_size + 4.0)
		label.size = Vector2(card_size.x, maxf(14.0, card_size.y - node_size - 18.0))
		tag.position = Vector2(0.0, card_size.y - 14.0)
		tag.size = Vector2(card_size.x, 12.0)
		centers[id] = card.position + button.position + button.size * 0.5
	tree_board.node_centers = centers
	tree_board.queue_redraw()


func _select_upgrade(id: String) -> void:
	if not upgrade_by_id.has(id):
		return
	selected_id = id
	var selected_data: Dictionary = upgrade_by_id[id]
	var state: String = _upgrade_state(selected_data)
	selected_title.text = str(selected_data["title"]).to_upper()
	selected_icon.texture = _load_upgrade_icon(selected_data)
	selected_status.text = _upgrade_tag(selected_data)
	selected_status.add_theme_color_override("font_color", _state_color(state))
	selected_cost.text = "Research cost  %s XP" % _format_number(int(selected_data["cost"]))
	selected_cost.add_theme_color_override("font_color", COLOR_GOLD if state != "locked" else COLOR_MUTED)
	selected_requires.text = _requirement_text(selected_data)
	selected_desc.text = _upgrade_description(selected_data)
	_update_unlock_button(selected_data)

	for id_key in node_cards:
		var card: Control = node_cards[id_key] as Control
		if card == null:
			continue
		var button := card.get_node("IconButton") as Button
		var card_state := _upgrade_state(upgrade_by_id[id_key])
		_style_upgrade_button(button, "selected" if id_key == selected_id else card_state)


func _refresh_tree() -> void:
	_update_xp_label()
	for id in node_cards:
		var upgrade: Dictionary = upgrade_by_id[id]
		var state: String = _upgrade_state(upgrade)
		tree_board.node_states[id] = state
		var card: Control = node_cards[id] as Control
		if card == null:
			continue
		var button := card.get_node("IconButton") as Button
		var label := card.get_node("NodeLabel") as Label
		var tag := card.get_node("NodeTag") as Label
		_style_upgrade_button(button, "selected" if id == selected_id else state)
		label.add_theme_color_override("font_color", COLOR_TEXT if state != "locked" else COLOR_MUTED)
		tag.text = _upgrade_tag(upgrade)
		tag.add_theme_color_override("font_color", _state_color(state))
	tree_board.queue_redraw()
	_select_upgrade(selected_id)


func _try_unlock_selected() -> void:
	if not upgrade_by_id.has(selected_id):
		return
	var upgrade: Dictionary = upgrade_by_id[selected_id]
	if _upgrade_state(upgrade) != "ready":
		return
	if bool(GameState.call("unlock_tech", selected_id, int(upgrade["cost"]), _requirements(upgrade))):
		_refresh_tree()


func _toggle_tech_effects() -> void:
	if not GameState.has_method("set_tech_effects_enabled"):
		return
	GameState.call("set_tech_effects_enabled", not _tech_effects_enabled())
	_refresh_effects_toggle_button()


func _refresh_effects_toggle_button() -> void:
	if effects_toggle_button == null:
		return
	var enabled := _tech_effects_enabled()
	effects_toggle_button.text = "EFFECTS\nON" if enabled else "EFFECTS\nOFF"
	effects_toggle_button.add_theme_color_override("font_color", COLOR_TEXT if enabled else COLOR_MUTED)
	effects_toggle_button.add_theme_color_override("font_hover_color", COLOR_TEXT)


func _tech_effects_enabled() -> bool:
	if not GameState.has_method("get_tech_effects_enabled"):
		return true
	return bool(GameState.call("get_tech_effects_enabled"))


func _update_unlock_button(upgrade: Dictionary) -> void:
	var state: String = _upgrade_state(upgrade)
	var cost: int = int(upgrade["cost"])
	unlock_button.disabled = state != "ready"
	match state:
		"unlocked", "master":
			unlock_button.text = "UNLOCKED"
		"ready":
			unlock_button.text = "UNLOCK\n%s XP" % _format_number(cost)
		"locked":
			if _requirements_met(upgrade):
				unlock_button.text = "NEED\n%s MORE" % _format_number(max(0, cost - _tech_xp()))
			else:
				unlock_button.text = "LOCKED"
		_:
			unlock_button.text = "LOCKED"


func _upgrade_state(upgrade: Dictionary) -> String:
	var id: String = str(upgrade["id"])
	if bool(GameState.call("has_tech", id)):
		return "master" if id == "apex_master" or id.ends_with("_apex") else "unlocked"
	if not _requirements_met(upgrade):
		return "locked"
	return "ready" if _tech_xp() >= int(upgrade["cost"]) else "locked"


func _upgrade_tag(upgrade: Dictionary) -> String:
	var state: String = _upgrade_state(upgrade)
	match state:
		"master":
			return "MASTER"
		"unlocked":
			return "UNLOCKED"
		"ready":
			return "READY"
		_:
			return "%s XP" % _format_number(int(upgrade["cost"])) if _requirements_met(upgrade) else "LOCKED"


func _requirement_text(upgrade: Dictionary) -> String:
	var reqs := _requirements(upgrade)
	if reqs.is_empty():
		return "No prerequisite"
	var names: Array[String] = []
	for req in reqs:
		var req_id := str(req)
		var req_upgrade: Dictionary = upgrade_by_id.get(req_id, {"title": req_id})
		names.append(str(req_upgrade.get("title", req_id)))
	return "Requires  %s" % " + ".join(names)


func _upgrade_description(upgrade: Dictionary) -> String:
	return "Tier %d - %s" % [_upgrade_tier(upgrade), str(upgrade.get("desc", ""))]


func _upgrade_tooltip(upgrade: Dictionary) -> String:
	return "%s\n%s\n%s" % [
		str(upgrade["title"]),
		_upgrade_description(upgrade),
		_requirement_text(upgrade),
	]


func _requirements_met(upgrade: Dictionary) -> bool:
	for req in _requirements(upgrade):
		if not bool(GameState.call("has_tech", str(req))):
			return false
	return true


func _requirements(upgrade: Dictionary) -> Array:
	var req = upgrade.get("req", [])
	if req is Array:
		return req
	return []


func _upgrade_tier(upgrade: Dictionary) -> int:
	return clampi(int(float(upgrade.get("col", 0))) + 1, 1, 4)


func _close() -> void:
	queue_free()


func _tech_xp() -> int:
	return max(0, int(GameState.get("tech_xp")))


func _update_xp_label() -> void:
	if xp_label != null:
		xp_label.text = "XP BANK: %s" % _format_number(_tech_xp())


func _apply_xp_label_font() -> void:
	if xp_label == null or SpaceTheme == null:
		return
	var font := load(str(SpaceTheme.get("FONT_BODY_PATH")))
	if font is Font:
		xp_label.add_theme_font_override("font", font)


func _format_number(value: int) -> String:
	var text := str(value)
	var out := ""
	var count := 0
	for i in range(text.length() - 1, -1, -1):
		if count > 0 and count % 3 == 0:
			out = "," + out
		out = text.substr(i, 1) + out
		count += 1
	return out


func _load_upgrade_icon(upgrade: Dictionary) -> Texture2D:
	var tower := str(upgrade["tower"])
	var fallback := str(TOWER_ICONS.get(tower, TOWER_ICONS["photon_splitter"]))
	return _load_texture(TECH_TIER_ICON_PATH % [tower, _upgrade_tier(upgrade)], fallback)


func _load_icon(tower: String) -> Texture2D:
	return _load_texture(str(TOWER_ICONS.get(tower, TOWER_ICONS["photon_splitter"])))


func _load_texture(path: String, fallback_path: String = "") -> Texture2D:
	var resource := load(path)
	if resource is Texture2D:
		return resource
	var image := Image.new()
	if image.load(path) == OK:
		return ImageTexture.create_from_image(image)
	if not fallback_path.is_empty() and fallback_path != path:
		return _load_texture(fallback_path)
	return null


func _state_color(state: String) -> Color:
	match state:
		"master", "selected":
			return COLOR_GOLD
		"unlocked":
			return Color(0.48, 1.0, 0.64, 0.98)
		"ready":
			return COLOR_CYAN
		_:
			return COLOR_MUTED


func _style_upgrade_button(button: Button, state: String) -> void:
	var accent := _state_color(state)
	var bg := Color(0.020, 0.052, 0.078, 0.98)
	var border_width := 1
	if state == "locked":
		bg = Color(0.014, 0.020, 0.032, 0.90)
	elif state == "selected":
		bg = Color(0.080, 0.068, 0.026, 0.98)
		border_width = 3
	elif state == "master":
		bg = Color(0.078, 0.056, 0.014, 0.98)
		border_width = 2
	button.add_theme_stylebox_override("normal", _button_style(bg, accent, 42.0, border_width))
	button.add_theme_stylebox_override("hover", _button_style(Color(bg.r + 0.020, bg.g + 0.030, bg.b + 0.040, 1.0), Color(accent.r, accent.g, accent.b, 1.0), 42.0, max(2, border_width)))
	button.add_theme_stylebox_override("pressed", _button_style(Color(0.052, 0.112, 0.138, 1.0), COLOR_GOLD, 42.0, 2))
	button.add_theme_stylebox_override("focus", _button_style(Color(0.015, 0.072, 0.092, 1.0), COLOR_GOLD, 42.0, 2))
	button.add_theme_color_override("font_color", COLOR_TEXT)
	button.add_theme_color_override("font_hover_color", COLOR_TEXT)
	button.add_theme_color_override("font_pressed_color", COLOR_TEXT)
	button.add_theme_color_override("font_focus_color", COLOR_TEXT)
	button.add_theme_constant_override("h_separation", 5)
	button.set_default_cursor_shape(Control.CURSOR_POINTING_HAND)
	button.modulate = Color(1.0, 1.0, 1.0, 0.72) if state == "locked" else Color.WHITE


func _panel_style(bg: Color, border: Color, radius: float, margin_x: float, margin_y: float) -> StyleBoxFlat:
	var style := StyleBoxFlat.new()
	style.bg_color = bg
	style.border_color = border
	style.set_border_width_all(1)
	style.set_corner_radius_all(int(radius))
	style.set_content_margin(SIDE_LEFT, margin_x)
	style.set_content_margin(SIDE_RIGHT, margin_x)
	style.set_content_margin(SIDE_TOP, margin_y)
	style.set_content_margin(SIDE_BOTTOM, margin_y)
	style.shadow_color = Color(0.0, 0.0, 0.0, 0.46)
	style.shadow_size = 9
	style.shadow_offset = Vector2(0.0, 2.0)
	return style


func _button_style(bg: Color, border: Color, radius: float, border_width: int) -> StyleBoxFlat:
	var style := _panel_style(bg, border, radius, 8.0, 8.0)
	style.set_border_width_all(border_width)
	style.shadow_size = 6
	return style

from pathlib import Path


router = (Path(__file__).resolve().parents[1] / "scripts/ui/main_menu_mode_router.gd").read_text(encoding="utf-8")

assert "const MODE_INFO" in router
for mode in ["campaign", "boss_rush", "daily_seed", "draft_defense", "endless", "no_flare"]:
    assert f'"{mode}"' in router
assert "mode_info_panel.offset_left = 310" in router
assert "button.mouse_entered.connect(_show_mode_info.bind(mode))" in router
assert "button.focus_entered.connect(_show_mode_info.bind(mode))" in router
assert "SpaceTheme.call(\"apply_deep_panel\", mode_info_panel, SpaceTheme.get(\"COLOR_GOLD\"))" in router

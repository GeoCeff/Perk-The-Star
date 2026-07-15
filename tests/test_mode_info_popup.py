from pathlib import Path


router = (Path(__file__).resolve().parents[1] / "gdextension/src/main_menu_native.cpp").read_text(encoding="utf-8")

assert "constexpr ModeInfo MODE_INFO" in router
for mode in ["campaign", "boss_rush", "daily_seed", "draft_defense", "endless", "no_flare"]:
    assert f'"{mode}"' in router
assert "mode_info_panel->set_offset(SIDE_LEFT, 310.0)" in router
assert 'button->connect("mouse_entered", Callable(this, "_show_mode_info").bind(mode))' in router
assert 'button->connect("focus_entered", Callable(this, "_show_mode_info").bind(mode))' in router
assert 'theme->call("apply_deep_panel", mode_info_panel, theme->get("COLOR_GOLD"))' in router

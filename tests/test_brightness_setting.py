from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


scene = read("scenes/ui/settings_overlay.tscn")
state_cpp = read("gdextension/src/game_state_native.cpp")
controls_cpp = read("gdextension/src/settings_controls_native.cpp")
register_cpp = read("gdextension/src/register_types.cpp")

assert '[node name="brightness_row" type="HBoxContainer"' in scene
assert 'type="MainMenuBrightnessSliderNative"' in scene
assert 'value_label_path = NodePath("../brightness_value")' in scene
assert 'ClassDB::register_class<MainMenuBrightnessSliderNative>();' in register_cpp
assert 'set_brightness' in state_cpp
assert 'display_settings_changed' in state_cpp
assert 'set_brightness", new_value / 100.0' in controls_cpp

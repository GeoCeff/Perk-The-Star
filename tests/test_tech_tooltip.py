from pathlib import Path


tech_tree = (Path(__file__).resolve().parents[1] / "gdextension/src/tech_tree_overlay_native.cpp").read_text(encoding="utf-8")

assert "class TechTreeTooltipButtonNative" in (Path(__file__).resolve().parents[1] / "gdextension/src/tech_tree_overlay_native.h").read_text(encoding="utf-8")
assert "Object* TechTreeTooltipButtonNative::_make_custom_tooltip" in tech_tree
assert "memnew(TechTreeTooltipButtonNative)" in tech_tree
assert "button->tooltip_data = upgrade_tooltip_data(upgrade)" in tech_tree
assert "upgrade_tooltip(" not in tech_tree

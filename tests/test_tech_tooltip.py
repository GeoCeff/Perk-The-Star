from pathlib import Path


tech_tree = (Path(__file__).resolve().parents[1] / "scripts/ui/tech_tree_overlay.gd").read_text(encoding="utf-8")

assert "class TechTreeTooltipButton" in tech_tree
assert "func _make_custom_tooltip" in tech_tree
assert "var button := TechTreeTooltipButton.new()" in tech_tree
assert "button.tooltip_data = _upgrade_tooltip_data(upgrade)" in tech_tree
assert "func _upgrade_tooltip(" not in tech_tree

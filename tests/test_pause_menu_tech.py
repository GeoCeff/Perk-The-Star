from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

pause_scene = (ROOT / "scenes/ui/game_pause_menu.tscn").read_text(encoding="utf-8")
pause_native = (ROOT / "gdextension/src/game_pause_menu_native.cpp").read_text(encoding="utf-8")
pause_header = (ROOT / "gdextension/src/game_pause_menu_native.h").read_text(encoding="utf-8")
hud_scene = (ROOT / "scenes/ui/game_hud.tscn").read_text(encoding="utf-8")
game_script = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")

assert "TechTreeButton" in pause_scene
assert 'text = "Tech Tree"' in pause_scene
assert "TECH_TREE_OVERLAY_SCENE_PATH" in pause_native
assert "_open_tech_tree" in pause_native
assert "open_tech_tree" in pause_header
assert "TechButton" not in hud_scene
assert "ActionRow/TechButton" not in game_script

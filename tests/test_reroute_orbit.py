from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
HUD_CPP = (ROOT / "gdextension/src/game_hud_native.cpp").read_text(encoding="utf-8")
HUD_H = (ROOT / "gdextension/src/game_hud_native.h").read_text(encoding="utf-8")


assert "REROUTE_ORBIT_COST" in GAME
assert "func _on_tower_reroute_requested" in GAME
assert "func _finish_reroute_from_screen_position" in GAME
assert '"tower_reroute_requested": Callable(self, "_on_tower_reroute_requested")' in GAME
assert "tower_reroute_requested" in HUD_CPP
assert "TowerManageRerouteButton" in HUD_CPP
assert "tower_manage_reroute_button" in HUD_H

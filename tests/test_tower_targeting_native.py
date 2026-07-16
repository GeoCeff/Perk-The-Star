from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
TOWER_LIB = (ROOT / "gdextension/src/game_tower_library_native.cpp").read_text(encoding="utf-8")


assert "can_damage_enemy" in TOWER_LIB
assert "target_index" in TOWER_LIB
assert 'variant == "mimic"' in TOWER_LIB
assert 'tower_library.call("target_index"' in GAME
assert 'tower_library.call("can_damage_enemy"' in GAME
assert "var best_sun_dist_squared" not in GAME

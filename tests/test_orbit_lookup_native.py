from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
ORBIT = (ROOT / "gdextension/src/game_orbit_math_native.cpp").read_text(encoding="utf-8")


assert "tower_index_for_slot" in ORBIT
assert "tower_index_at_world_position" in ORBIT
assert 'orbit_math.call("tower_index_for_slot"' in GAME
assert 'orbit_math.call("tower_index_at_world_position"' in GAME
assert "var best_dist_squared: float = INF" not in GAME

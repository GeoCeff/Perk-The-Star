from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
GAMEPLAY_MATH = (ROOT / "gdextension/src/v2_gameplay_math.cpp").read_text(encoding="utf-8")


assert "V2GameplayMath::spawn_position_for_pattern" in GAMEPLAY_MATH
assert 'gameplay_math.call("spawn_position_for_pattern"' in GAME
assert "var normalized_pattern" not in GAME
assert 'match normalized_pattern' not in GAME

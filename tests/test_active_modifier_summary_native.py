from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
RUNTIME = (ROOT / "gdextension/src/game_runtime_native.cpp").read_text(encoding="utf-8")


assert "active_modifier_summary" in RUNTIME
assert "Cryo offline" in RUNTIME
assert "Bio %.0fx" in RUNTIME
assert "Dark %s" in RUNTIME
assert 'runtime_native.call("active_modifier_summary"' in GAME
assert "var ring_parts: Array" not in GAME

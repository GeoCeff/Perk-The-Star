from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
RUNTIME = (ROOT / "gdextension/src/game_runtime_native.cpp").read_text(encoding="utf-8")


assert "end_state_view_data" in RUNTIME
assert "ENDLESS RUN ENDED" in RUNTIME
assert "TECH XP +" in RUNTIME
assert 'runtime_native.call("end_state_view_data"' in GAME
assert 'title = "ENDLESS RUN ENDED"' not in GAME
assert 'stats += "  |  TECH XP' not in GAME

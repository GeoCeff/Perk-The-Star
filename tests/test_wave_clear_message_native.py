from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
RUNTIME = (ROOT / "gdextension/src/game_runtime_native.cpp").read_text(encoding="utf-8")


assert "wave_clear_message" in RUNTIME
assert "Corps reward" in RUNTIME
assert "Perfect Orbit" in RUNTIME
assert 'runtime_native.call("wave_clear_message"' in GAME
assert 'var text: String = "%s. Corps reward' not in GAME

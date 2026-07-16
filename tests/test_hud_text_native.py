from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
RUNTIME = (ROOT / "gdextension/src/game_runtime_native.cpp").read_text(encoding="utf-8")


assert "hud_text_state" in RUNTIME
assert "DRAFT DEFENSE | PICK CONTRACT" in RUNTIME
assert "OVER\\nARMED" in RUNTIME
assert 'runtime_native.call("hud_text_state"' in GAME
assert 'title_text = "ENDLESS WAVE' not in GAME
assert 'overcharge_text = "OVER\\n' not in GAME

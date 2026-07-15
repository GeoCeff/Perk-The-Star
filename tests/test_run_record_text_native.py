from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
RUNTIME = (ROOT / "gdextension/src/game_runtime_native.cpp").read_text(encoding="utf-8")


assert "run_record_text" in RUNTIME
assert "BOSS RUSH: WAVE" in RUNTIME
assert 'runtime_native.call("run_record_text"' in GAME
assert 'match str(summary.get("mode", run_mode))' not in GAME

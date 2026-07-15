from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
RUNTIME = (ROOT / "gdextension/src/game_runtime_native.cpp").read_text(encoding="utf-8")
PROMPTS = (ROOT / "docs/GAME_UPDATE_PROMPTS.md").read_text(encoding="utf-8")


assert "run_tech_xp_award" in RUNTIME
assert 'run_mode == "boss_rush"' in RUNTIME
assert 'xp_parts.append(vformat("VICTORY %d", victory_bonus))' in RUNTIME
assert 'runtime_native.call(\n\t\t"run_tech_xp_award"' in GAME
assert "var score_xp" not in GAME
assert "GameRuntimeNative::run_tech_xp_award" in PROMPTS

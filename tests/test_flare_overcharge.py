from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
RUNTIME = (ROOT / "gdextension/src/game_runtime_native.cpp").read_text(encoding="utf-8")


assert "var flare_overcharged: bool = false" in GAME
assert 'Callable(self, "_try_overcharge_flare")' in GAME
assert "func _try_overcharge_flare() -> void:" in GAME
assert "FLARE_OVERCHARGE_COST" in GAME
assert "FLARE_OVERCHARGE_DAMAGE_BONUS" in GAME
assert 'data["overcharge_disabled"] = overcharge_disabled' in RUNTIME

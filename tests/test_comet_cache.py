from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
WAVE_08 = (ROOT / "data/waves/wave_08.json").read_text(encoding="utf-8")
WAVE_LIB = (ROOT / "gdextension/src/game_wave_library_native.cpp").read_text(encoding="utf-8")


assert 'COMET_CACHE_VARIANT: String = "comet_cache"' in GAME
assert 'func _spawn_comet_cache(event: Dictionary) -> void:' in GAME
assert '"type"] = "comet_cache"' in WAVE_LIB
assert '"type": "comet_cache"' in WAVE_08
assert "CACHE" in WAVE_LIB
assert "Destroy Comet Cache for Sol" in WAVE_LIB

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
WAVE_LIB = (ROOT / "gdextension/src/game_wave_library_native.cpp").read_text(encoding="utf-8")


assert "banner_data" in WAVE_LIB
assert 'wave_type == "formation"' in WAVE_LIB
assert 'wave_library.call("banner_data"' in GAME
assert 'match str(next_data.get("wave_type", "normal"))' not in GAME

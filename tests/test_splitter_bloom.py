from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
WAVE_11 = (ROOT / "data/waves/wave_11.json").read_text(encoding="utf-8")
WAVE_LIB = (ROOT / "gdextension/src/game_wave_library_native.cpp").read_text(encoding="utf-8")


assert 'SPLITTER_BLOOM_VARIANT: String = "splitter_bloom"' in GAME
assert 'cfg_key: String = "bloom" if key == SPLITTER_BLOOM_VARIANT else key' in GAME
assert "split_count: int = 2 if variant == SPLITTER_BLOOM_VARIANT else 3" in GAME
assert '"variant": "splitter_bloom"' in WAVE_11
assert 'keys.append("prime");\n    keys.append("splitter_bloom");' in WAVE_LIB
assert "Splitter Bloom" in WAVE_LIB

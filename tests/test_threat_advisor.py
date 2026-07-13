from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
WAVE_LIB = (ROOT / "gdextension/src/game_wave_library_native.cpp").read_text(encoding="utf-8")


for wave in range(1, 7):
    text = (ROOT / f"data/waves/wave_{wave:02d}.json").read_text(encoding="utf-8")
    assert "Threat Advisor:" in text

assert "ADVISOR: Photon Splitters handle Drifters" in WAVE_LIB
assert "ADVISOR: Mimics ignore Photon" in WAVE_LIB
assert "EVENT: Solar storm fires a flare" in WAVE_LIB

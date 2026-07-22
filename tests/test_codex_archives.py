from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CODEX = (ROOT / "gdextension/src/codex_native.cpp").read_text(encoding="utf-8")
SCENE = (ROOT / "scenes/ui/codex.tscn").read_text(encoding="utf-8")

for key in ["challenges", "tech_tree"]:
    assert f'nav_buttons["{key}"]' in CODEX
    assert f'sections["{key}"]' in CODEX
    assert f'name="btn_{key}"' in SCENE

for topic in ["Boss Rush", "Daily Seed", "Draft Defense", "Endless Defense", "No-Flare Challenge", "Apex doctrine"]:
    assert topic in CODEX

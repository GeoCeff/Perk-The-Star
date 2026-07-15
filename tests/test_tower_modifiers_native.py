from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
TOWER_LIB = (ROOT / "gdextension/src/game_tower_library_native.cpp").read_text(encoding="utf-8")


assert "runtime_stats_with_modifiers" in TOWER_LIB
assert 'has_string(unlocked_tech, "solar_lens")' in TOWER_LIB
assert 'apex_id = "tardigrade_apex"' in TOWER_LIB
assert "has_string(unlocked_tech, apex_id)" in TOWER_LIB
assert 'draft_towers.has(tower_type)' in TOWER_LIB
assert "_apply_draft_package_stats" not in GAME
assert 'if _has_tech("solar_lens")' not in GAME

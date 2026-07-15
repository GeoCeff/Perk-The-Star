from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
TOWER_LIB = (ROOT / "gdextension/src/game_tower_library_native.cpp").read_text(encoding="utf-8")


assert "runtime_stats_with_modifiers" in TOWER_LIB
assert "tech_slow_duration" in TOWER_LIB
assert "slingshot_cost" in TOWER_LIB
assert 'has_string(unlocked_tech, "solar_lens")' in TOWER_LIB
assert 'apex_id = "tardigrade_apex"' in TOWER_LIB
assert "has_string(unlocked_tech, apex_id)" in TOWER_LIB
assert 'draft_towers.has(tower_type)' in TOWER_LIB
assert 'has_string(unlocked_tech, "slingshot_coils") ? 30' in TOWER_LIB
assert "_apply_draft_package_stats" not in GAME
assert 'if _has_tech("solar_lens")' not in GAME
assert 'return 30 if _has_tech("slingshot_coils") else SLINGSHOT_COST' not in GAME

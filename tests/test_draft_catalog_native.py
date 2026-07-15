from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
CATALOG = (ROOT / "gdextension/src/game_catalog_native.cpp").read_text(encoding="utf-8")


assert "draft_packages" in CATALOG
assert '"Sol Windfall"' in CATALOG
assert '"Bio Bloom"' in CATALOG
assert 'game_catalog.call("draft_packages")' in GAME
assert '"Sol Windfall"' not in GAME

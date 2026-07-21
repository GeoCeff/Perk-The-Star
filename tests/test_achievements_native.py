from pathlib import Path
from xml.etree import ElementTree


ROOT = Path(__file__).resolve().parents[1]
STATE = (ROOT / "gdextension/src/game_state_native.cpp").read_text(encoding="utf-8")
MENU = (ROOT / "gdextension/src/main_menu_native.cpp").read_text(encoding="utf-8")
GAME = (ROOT / "scripts/game/game.gd").read_text(encoding="utf-8")
SCENE = (ROOT / "scenes/main_menu.tscn").read_text(encoding="utf-8")
ICONS = ROOT / "assets/ui/icons/achievement_emblems.svg"


for achievement_id in [
    "first_light",
    "swarm_warranty",
    "compound_starlight",
    "chain_of_command",
    "perfect_attendance",
    "no_dimmers",
    "last_photon",
    "flareless_future",
    "prime_time_over",
    "same_sun",
    "drafted_greatness",
    "one_more_orbit",
]:
    assert f'{{"{achievement_id}"' in STATE

assert 'summary["new_achievements"] = update_achievements' in STATE
assert 'config->set_value("achievements", "unlocked"' in STATE
assert 'GameState.call(\n\t\t\t"record_run"' in GAME
assert '"achievement_text": _achievement_unlock_text()' in GAME
assert 'name="btn_achievements"' in SCENE
assert 'state->call("achievement_board")' in MENU
assert ElementTree.parse(ICONS).getroot().attrib["viewBox"] == "0 0 768 64"

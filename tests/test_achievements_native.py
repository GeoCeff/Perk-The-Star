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
assert 'config->set_value("achievements", "tiers"' in STATE
assert 'config->set_value("achievements", "mode_victories"' in STATE
assert '"first_light", "First Light"' in STATE and 'TOTAL_WAVES, 1, 25, 75' in STATE
assert '"last_photon", "The Last Photon"' in STATE and 'LAST_LIGHT_VICTORY, 30, 20, 10' in STATE
assert '"same_sun", "Same Sun, Same Trouble"' in STATE and 'MODE_VICTORY, 1, 3, 7' in STATE
assert 'item["tier_level"] = tier' in STATE
assert 'item["medal_color"] = TIER_COLORS[tier]' in STATE
assert '"%s MEDAL  -  %s"' in STATE
assert 'GameState.call(\n\t\t\t"record_run"' in GAME
assert '"achievement_text": _achievement_unlock_text()' in GAME
assert '"MEDAL COMMISSIONED: %s"' in GAME
assert 'name="btn_achievements"' in SCENE
assert 'state->call("achievement_board")' in MENU
assert 'ClassDB::instantiate("MainMenuFxNative")' in MENU
assert 'frame->set("frame_target_path", NodePath("../AchievementsBoard"))' in MENU
assert 'kicker->set_text("SOL COMMAND  //  COMMANDER\'S MEDAL VAULT")' in MENU
assert 'title->set_text("CONSTELLATION OF VALOR")' in MENU
assert 'medal_column->set_name("ServiceMedal")' in MENU
assert 'glint->set_text(String::chr(0x2726))' in MENU
assert 'shine->set_loops()' in MENU
assert '"BRONZE COMMISSION"' in MENU and '"SILVER COMMISSION"' in MENU and '"GOLD COMMISSION"' in MENU
assert ElementTree.parse(ICONS).getroot().attrib["viewBox"] == "0 0 768 64"

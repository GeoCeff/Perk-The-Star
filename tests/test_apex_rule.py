from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[1]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


tech_tree = read("scripts/ui/tech_tree_overlay.gd")
apex_ids = re.search(r"const APEX_TECH_IDS := \[(.*?)\]", tech_tree).group(1)
apex_ids = re.findall(r'"([^"]+)"', apex_ids)

assert apex_ids == [
    "photon_apex",
    "cryo_apex",
    "bio_apex",
    "magnetic_apex",
    "helios_apex",
    "tardigrade_apex",
]
assert 'APEX_ICON_PATH := "res://assets/ui/tech_tiers/apex_tier.png"' in tech_tree
assert '"apex_master"' not in tech_tree
assert '_has_tech("apex_master")' not in read("scripts/game/game.gd")
assert 'sprite("Apex", "res://assets/ui/tech_tiers/apex_tier.png")' in read("gdextension/src/codex_native.cpp")
assert 'tech_id == "apex_master"' in read("gdextension/src/game_state_native.cpp")

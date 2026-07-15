# Perk the Star Update Prompt Bank

Copy one prompt at a time. These are written for the existing Godot 4.6 + GDExtension project.

## Project Ground Rules

- Work in `D:\Code\Projects\app-projects\perk-the-star`.
- Reuse existing systems first: `scripts/game/game.gd`, `data/waves/*.json`, `GameCatalogNative`, `GameTowerLibraryNative`, `GameWaveLibraryNative`, `GameStateNative`, `GameHudNative`, and the existing scenes.
- Prefer wave JSON, balance constants, HUD text, and existing tower/enemy behavior before adding new systems.
- If C++ changes, rebuild with `scons platform=windows target=template_debug arch=x86_64`.

## Fast Prompts

1. Inspect Perk the Star and improve the first 5 minutes of play. Focus on tower choice, Wave 1-3 pacing, Wave Intel clarity, and whether the player understands orbital slots. Keep the diff small and run one quick check.

2. Make Campaign Wave 1 teach the core loop better using only `data/waves/wave_01.json`, HUD messages, and existing tutorial/brief text. Do not add a new tutorial system.

3. Improve the Wave Intel panel so it explains the next threat more clearly for Drifters, Blooms, Burrowers, Mimics, Farmers, and Prime. Reuse `GameWaveLibraryNative` formatting.

4. Tune early Sol Credits, tower costs, or wave rewards so the player can make at least two meaningful tower choices before Wave 3 without making Campaign too easy.

5. Make tower placement feedback clearer: build preview, occupied slots, insufficient Sol, selected tower readout, and placed-tower feedback. Use existing HUD/SFX/effects.

6. Improve the end screen so Campaign, Endless, and No-Flare each clearly show why the run mattered: Tech XP, best record, rank, waves cleared, and next suggested action.

7. Add one tiny quality-of-life improvement to controls. Candidates: clearer Helios Slingshot hint, hotkey reminder, better recenter feedback, or safer click handling near HUD.

8. Make the Tech Tree easier to understand without redesigning it. Improve selected-node text, requirements, affordability state, and tower-path labels in `tech_tree_overlay_native.cpp`.

9. Improve No-Flare Challenge messaging so it feels like a real mode, not Campaign with a disabled button. Update menu/mode brief, end text, and Tech XP explanation.

10. Playtest mentally from Wave 1 to Wave 4 and fix the smallest real friction point you find. Report what you changed and what you deliberately skipped.

## Gameplay Ideas

11. Add a Boss Rush mode using existing Prime, clash groups, and scaling logic. Start from a compact menu route, reuse current end screen records if practical, and avoid new enemy art.

12. Add a Daily Seed mode using deterministic wave generation based on the date. Reuse Endless scaling and store only the best score/wave for that date if saving is already easy.

13. Add Draft Defense: before Wave 1, offer three random tower loadouts or tech perks, then run a short 6-wave mission. Reuse existing towers and Tech XP unlock effects.

14. Add a "Solar Storm" wave modifier: one ring is temporarily blinded or slowed, using the existing `ring_blind` event style. Add it first to Endless every 8 waves.

15. Add a "Comet Cache" risk/reward event: a temporary target or pickup appears outside the outer ring; destroying it grants Sol or Tech XP, but ignoring it spawns extra Drifters.

16. Add a "Perfect Orbit" bonus for clearing a wave without losing luminosity. Reward small Sol Credits or score. Show it in the wave-clear message and end stats.

17. Add a combo score multiplier for fast enemy kills during active waves. Keep it visual-only plus score; do not let it break Sol economy.

18. Add a Prime Echo mini-boss every 10 Endless waves using the existing Prime sprite and lower HP. Reuse `Prime Echo` naming already implied in Endless.

19. Add a new enemy behavior without new art: "Splitter Bloom" divides into two weak Drifters on death. Implement it in the shared enemy death path and add one late wave test.

20. Add a new enemy behavior without new art: "Shielded Mimic" ignores Photon Splitter and has a temporary shield that Bio-Lab can crack faster.

21. Add a new enemy behavior without new art: "Flaring Farmer" gains speed from Helios hits but drops extra Sol if killed while slowed.

22. Add a new tower-side mechanic: Cryo + Magnetic slow synergy makes slowed enemies briefly brittle. Reuse or extend the existing `brittle_shells` logic.

23. Add a new tower-side mechanic: Photon Splitters can chain once at level 3. Keep it readable and cap it to one extra target.

24. Add a new tower-side mechanic: Tardigrade Bomb leaves a short lingering spore field. Reuse existing visual effects and slow/damage timers if possible.

25. Add a new Helios Slingshot upgrade interaction: if the shot crosses two rings before impact, it earns bonus damage or a small Sol refund.

26. Add ring identity bonuses: Corona Belt favors Photon/Helios, Chromosphere favors Cryo/Tardigrade, Photosphere favors Bio/Magnetic, Outer Veil favors early intercept. Keep bonuses small and visible.

27. Add a between-wave "reroute orbit" action: move one tower to an empty slot for a Sol cost. Keep it between-wave only and reuse tower management UI.

28. Add a salvage decision after each boss/clash wave: choose Sol now, Tech XP later, or one temporary tower buff next wave. Implement one simple choice UI if existing HUD can support it.

29. Add a "Solar Flare Overcharge" option: spend Sol to boost a ready flare, but only outside No-Flare mode. Keep damage and cost in catalog/state constants.

30. Add "Apex Master" as a final expensive tech unlocked after all six apex upgrades. The existing code already checks `apex_master`; wire it into the Tech Tree.

## Wave And Balance Prompts

31. Review all 12 campaign wave JSON files and make the difficulty ramp smoother. Change only wave data unless a code bug blocks good tuning.

32. Make Wave 12 feel more like a finale. Tune Prime timing, escort groups, Wave Intel, boss music timing, and victory text using existing systems.

33. Make Wave 6 the first real skill check. Use a clear enemy mix, one event, and enough reward afterward to recover.

34. Add one new campaign wave variant pattern using existing `ring`, `v_shape`, `spiral`, `center_top`, and `random` spawn logic. Prefer data changes first.

35. Tune Endless so waves 1-5 onboard, 6-12 pressure, 13+ scale, and every 5th/7th/10th wave feels different. Keep formulas readable in `_endless_wave_data`.

36. Balance Tech XP earnings so a failed early run still unlocks something soon, but apex upgrades still require repeat play. Adjust `_award_run_tech_xp_once` only if needed.

37. Audit tower costs and upgrade costs across Photon, Cryo, Bio-Lab, Magnetic, Helios, and Tardigrade. Make one focused balance pass and explain the intended role of each.

38. Make Solar Farmers, Photon Mimics, and Coronal Burrowers clearer threats. Improve their Wave Intel text and hit/interaction feedback before changing stats.

39. Add a small test-start/dev flow for quickly launching Wave 12 or Endless Wave 10 if it already fits `enable_test_run`. Keep it out of normal player UI.

40. Make No-Flare rewards fair. Tune its Tech XP bonus and end-screen copy so the mode feels worth replaying.

## Polish Prompts

41. Improve game feel for hits and kills: enemy hit flash, floating text, screen shake, SFX calls, and shot visuals. Use existing `GameEffectStoreNative` and SFX bus.

42. Improve orbital readability at 1920x1080: ring contrast, slot markers, tower icons, enemy health bars, and build preview. Keep the space background visible.

43. Improve pause/settings polish: screen shake toggle, auto-start toggle, music controls, and return flow. Reuse current settings storage.

44. Improve the main menu mode routing. Campaign, Endless, No-Flare, Tech Tree, Codex, Settings, and Credits should all feel intentional and consistent.

45. Improve Codex content so it explains enemy weaknesses, tower roles, rings, Tech XP, and modes using current project terms. Keep it concise.

46. Improve audio moments: wave start, wave clear, Prime entrance, Solar Flare, Slingshot, Tech unlock, insufficient Sol, and game over. Use existing SFX assets or fallback tones.

47. Add accessibility basics: stronger non-color indicators for tower state, reduced-motion respect for screen shake, readable tooltip sizes, and keyboard access for key overlays.

48. Make HUD text fit safely on smaller windows. Check long wave names, end stats, tower management cards, and Tech Tree selected descriptions.

49. Clean one obvious duplicated or stale asset/code path only if it is unused by the current project. Do not refactor gameplay while doing cleanup.

50. Update `README.md` with any new mode/mechanic you add, but only the run controls and file-map parts that actually changed.

## Big Update Prompts

51. Build a small Version 2 update: add Boss Rush, one new Endless modifier, one Tech Tree upgrade, and a polish pass on end screens. Keep it shippable in one session.

52. Build a replayability update: Daily Seed, Perfect Orbit bonuses, clearer best records, and one Endless tuning pass. Reuse existing save settings.

53. Build a tower identity update: one small special interaction for each tower, visible tower info text, and a balance pass. Avoid new art unless already available.

54. Build a Prime finale update: better Wave 12 staging, Prime phase feedback, boss SFX/music timing, and victory/defeat copy.

55. Build a new-player update: clearer first wave, better tower role hints, improved tutorial skip/complete state, and stronger Wave Intel.

56. Build a challenge-player update: No-Flare polish, Endless records, optional harder modifiers, and better rewards for high-luminosity clears.

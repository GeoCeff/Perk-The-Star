#include "game_state_native.h"

#include <godot_cpp/classes/canvas_layer.hpp>
#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace {

bool is_apex_tech(const String& tech_id) {
    return tech_id.ends_with("_apex");
}

bool has_any_apex_tech(const Array& unlocked_tech) {
    for (int i = 0; i < unlocked_tech.size(); ++i) {
        if (is_apex_tech(String(unlocked_tech[i]))) {
            return true;
        }
    }
    return false;
}

enum AchievementRule {
    TOTAL_WAVES,
    TOTAL_KILLS,
    TOTAL_SCORE,
    BEST_COMBO,
    BEST_PERFECT_ORBITS,
    CAMPAIGN_FULL_SHINE,
    LAST_LIGHT_VICTORY,
    MODE_VICTORY,
    ENDLESS_WAVES,
};

struct AchievementInfo {
    const char* id;
    const char* title;
    const char* description;
    AchievementRule rule;
    int bronze_target;
    int silver_target;
    int gold_target;
    const char* mode;
    const char* unit;
    int icon_index;
    Color accent;
    bool lower_is_better = false;
};

const AchievementInfo ACHIEVEMENTS[] = {
    {"first_light", "First Light", "Hold the line across enough waves to earn a command star.", TOTAL_WAVES, 1, 25, 75, "", "WAVES", 0, Color(1.0, 0.76, 0.24)},
    {"swarm_warranty", "Void the Swarm Warranty", "Erase Astrophages until their warranty department stops answering.", TOTAL_KILLS, 250, 1000, 2500, "", "KILLS", 1, Color(0.38, 0.92, 1.0)},
    {"compound_starlight", "Starlight Has Compound Interest", "Bank a career fortune in light, heat, and impossible arithmetic.", TOTAL_SCORE, 100000, 350000, 1000000, "", "SCORE", 2, Color(0.62, 0.88, 1.0)},
    {"chain_of_command", "Chain of Command", "Forge an unbroken combo worthy of the SOL command ledger.", BEST_COMBO, 10, 15, 25, "", "COMBO", 3, Color(0.42, 0.96, 0.82)},
    {"perfect_attendance", "Perfect Attendance", "String together flawless orbits while the whole system watches.", BEST_PERFECT_ORBITS, 4, 8, 12, "", "PERFECT ORBITS", 4, Color(0.44, 1.0, 0.62)},
    {"no_dimmers", "No Dimmers Installed", "Win Normal Defense with enough sunlight to embarrass a supernova.", CAMPAIGN_FULL_SHINE, 80, 95, 100, "campaign", "LIGHT", 5, Color(1.0, 0.88, 0.34)},
    {"last_photon", "The Last Photon", "Win on a dying ember. The dimmer the star, the brighter the legend.", LAST_LIGHT_VICTORY, 30, 20, 10, "", "LIGHT", 6, Color(1.0, 0.46, 0.58), true},
    {"flareless_future", "Flares? Where We're Going...", "Repeat No-Flare victories until restraint becomes doctrine.", MODE_VICTORY, 1, 3, 5, "no_flare", "VICTORIES", 7, Color(0.94, 0.42, 0.42)},
    {"prime_time_over", "Prime Time Is Over", "Send the Boss Rush back into permanent reruns.", MODE_VICTORY, 1, 3, 5, "boss_rush", "VICTORIES", 8, Color(1.0, 0.52, 0.26)},
    {"same_sun", "Same Sun, Same Trouble", "Master the Daily Seed until fate starts feeling predictable.", MODE_VICTORY, 1, 3, 7, "daily_seed", "VICTORIES", 9, Color(0.38, 0.86, 1.0)},
    {"drafted_greatness", "Drafted Into Greatness", "Keep winning with whatever contract command slides across the desk.", MODE_VICTORY, 1, 3, 5, "draft_defense", "VICTORIES", 10, Color(0.72, 0.62, 1.0)},
    {"one_more_orbit", "One More Orbit", "Outlast Endless Defense until the clock files for reassignment.", ENDLESS_WAVES, 20, 30, 40, "endless", "WAVES", 11, Color(0.34, 0.94, 0.92)},
};

}

void GameStateNative::_bind_methods() {
    ClassDB::bind_method(D_METHOD("reset_state"), &GameStateNative::reset_state);
    ClassDB::bind_method(D_METHOD("load_audio_settings"), &GameStateNative::load_audio_settings);
    ClassDB::bind_method(D_METHOD("save_audio_settings"), &GameStateNative::save_audio_settings);
    ClassDB::bind_method(D_METHOD("ensure_music_audible"), &GameStateNative::ensure_music_audible);
    ClassDB::bind_method(D_METHOD("set_music_enabled", "enabled"), &GameStateNative::set_music_enabled);
    ClassDB::bind_method(D_METHOD("set_music_volume", "volume"), &GameStateNative::set_music_volume);
    ClassDB::bind_method(D_METHOD("get_music_volume_db"), &GameStateNative::get_music_volume_db);
    ClassDB::bind_method(D_METHOD("set_brightness", "value"), &GameStateNative::set_brightness);
    ClassDB::bind_method(D_METHOD("set_tutorial_completed", "completed"), &GameStateNative::set_tutorial_completed, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("set_screen_shake_enabled", "enabled"), &GameStateNative::set_screen_shake_enabled);
    ClassDB::bind_method(D_METHOD("set_auto_start_waves_enabled", "enabled"), &GameStateNative::set_auto_start_waves_enabled);
    ClassDB::bind_method(D_METHOD("set_tech_effects_enabled", "enabled"), &GameStateNative::set_tech_effects_enabled);
    ClassDB::bind_method(D_METHOD("add_tech_xp", "amount"), &GameStateNative::add_tech_xp);
    ClassDB::bind_method(D_METHOD("unlock_tech", "tech_id", "cost", "requirements"), &GameStateNative::unlock_tech, DEFVAL(Array()));
    ClassDB::bind_method(D_METHOD("has_tech", "tech_id"), &GameStateNative::has_tech);
    ClassDB::bind_method(D_METHOD("record_run", "run_mode", "score", "waves", "luminosity_percent", "rank", "victory", "perfect_orbits", "best_combo"), &GameStateNative::record_run);
    ClassDB::bind_method(D_METHOD("best_run_summary", "run_mode"), &GameStateNative::best_run_summary);
    ClassDB::bind_method(D_METHOD("achievement_board"), &GameStateNative::achievement_board);
    ClassDB::bind_method(D_METHOD("enable_test_run", "start_wave"), &GameStateNative::enable_test_run);
    ClassDB::bind_method(D_METHOD("clear_test_run"), &GameStateNative::clear_test_run);
    ClassDB::bind_method(D_METHOD("consume_test_start_wave"), &GameStateNative::consume_test_start_wave);
    ClassDB::bind_method(D_METHOD("damage_sun", "amount"), &GameStateNative::damage_sun);
    ClassDB::bind_method(D_METHOD("get_luminosity_percent"), &GameStateNative::get_luminosity_percent);
    ClassDB::bind_method(D_METHOD("add_credits", "amount"), &GameStateNative::add_credits);
    ClassDB::bind_method(D_METHOD("spend_credits", "amount"), &GameStateNative::spend_credits);
    ClassDB::bind_method(D_METHOD("can_afford", "amount"), &GameStateNative::can_afford);
    ClassDB::bind_method(D_METHOD("get_tower_cost", "tower_type"), &GameStateNative::get_tower_cost);
    ClassDB::bind_method(D_METHOD("get_upgrade_cost", "tower_type"), &GameStateNative::get_upgrade_cost);
    ClassDB::bind_method(D_METHOD("add_score", "amount"), &GameStateNative::add_score);
    ClassDB::bind_method(D_METHOD("on_enemy_killed", "variant_id"), &GameStateNative::on_enemy_killed);
    ClassDB::bind_method(D_METHOD("on_wave_cleared"), &GameStateNative::on_wave_cleared);
    ClassDB::bind_method(D_METHOD("try_trigger_flare"), &GameStateNative::try_trigger_flare);
    ClassDB::bind_method(D_METHOD("add_burrower"), &GameStateNative::add_burrower);
    ClassDB::bind_method(D_METHOD("remove_burrower"), &GameStateNative::remove_burrower);
    ClassDB::bind_method(D_METHOD("set_phase", "new_phase"), &GameStateNative::set_phase);
    ClassDB::bind_method(D_METHOD("get_rank"), &GameStateNative::get_rank);
    ClassDB::bind_method(D_METHOD("trigger_victory"), &GameStateNative::trigger_victory);
    ClassDB::bind_method(D_METHOD("get_phase"), &GameStateNative::get_phase);
    ClassDB::bind_method(D_METHOD("get_menu_phase"), &GameStateNative::get_menu_phase);
    ClassDB::bind_method(D_METHOD("get_between_wave_phase"), &GameStateNative::get_between_wave_phase);
    ClassDB::bind_method(D_METHOD("get_wave_active_phase"), &GameStateNative::get_wave_active_phase);
    ClassDB::bind_method(D_METHOD("get_paused_phase"), &GameStateNative::get_paused_phase);
    ClassDB::bind_method(D_METHOD("get_game_over_phase"), &GameStateNative::get_game_over_phase);
    ClassDB::bind_method(D_METHOD("get_victory_phase"), &GameStateNative::get_victory_phase);

    ClassDB::bind_method(D_METHOD("set_luminosity", "value"), &GameStateNative::set_luminosity);
    ClassDB::bind_method(D_METHOD("get_luminosity"), &GameStateNative::get_luminosity);
    ClassDB::bind_method(D_METHOD("set_sol_credits", "value"), &GameStateNative::set_sol_credits);
    ClassDB::bind_method(D_METHOD("get_sol_credits"), &GameStateNative::get_sol_credits);
    ClassDB::bind_method(D_METHOD("set_current_wave", "value"), &GameStateNative::set_current_wave);
    ClassDB::bind_method(D_METHOD("get_current_wave"), &GameStateNative::get_current_wave);
    ClassDB::bind_method(D_METHOD("set_flare_charge", "value"), &GameStateNative::set_flare_charge);
    ClassDB::bind_method(D_METHOD("get_flare_charge"), &GameStateNative::get_flare_charge);
    ClassDB::bind_method(D_METHOD("set_waves_since_last_flare", "value"), &GameStateNative::set_waves_since_last_flare);
    ClassDB::bind_method(D_METHOD("get_waves_since_last_flare"), &GameStateNative::get_waves_since_last_flare);
    ClassDB::bind_method(D_METHOD("set_performance_score", "value"), &GameStateNative::set_performance_score);
    ClassDB::bind_method(D_METHOD("get_performance_score"), &GameStateNative::get_performance_score);
    ClassDB::bind_method(D_METHOD("set_enemies_killed_total", "value"), &GameStateNative::set_enemies_killed_total);
    ClassDB::bind_method(D_METHOD("get_enemies_killed_total"), &GameStateNative::get_enemies_killed_total);
    ClassDB::bind_method(D_METHOD("set_waves_cleared", "value"), &GameStateNative::set_waves_cleared);
    ClassDB::bind_method(D_METHOD("get_waves_cleared"), &GameStateNative::get_waves_cleared);
    ClassDB::bind_method(D_METHOD("set_burrowers_active", "value"), &GameStateNative::set_burrowers_active);
    ClassDB::bind_method(D_METHOD("get_burrowers_active"), &GameStateNative::get_burrowers_active);
    ClassDB::bind_method(D_METHOD("get_music_enabled"), &GameStateNative::get_music_enabled);
    ClassDB::bind_method(D_METHOD("get_music_volume"), &GameStateNative::get_music_volume);
    ClassDB::bind_method(D_METHOD("get_brightness"), &GameStateNative::get_brightness);
    ClassDB::bind_method(D_METHOD("get_tutorial_completed"), &GameStateNative::get_tutorial_completed);
    ClassDB::bind_method(D_METHOD("get_screen_shake_enabled"), &GameStateNative::get_screen_shake_enabled);
    ClassDB::bind_method(D_METHOD("get_auto_start_waves_enabled"), &GameStateNative::get_auto_start_waves_enabled);
    ClassDB::bind_method(D_METHOD("get_tech_effects_enabled"), &GameStateNative::get_tech_effects_enabled);
    ClassDB::bind_method(D_METHOD("get_tech_xp"), &GameStateNative::get_tech_xp);
    ClassDB::bind_method(D_METHOD("get_unlocked_tech"), &GameStateNative::get_unlocked_tech);
    ClassDB::bind_method(D_METHOD("get_test_unlimited_sol_enabled"), &GameStateNative::get_test_unlimited_sol_enabled);
    ClassDB::bind_method(D_METHOD("get_music_changed_by_user_this_session"), &GameStateNative::get_music_changed_by_user_this_session);
    ClassDB::bind_method(D_METHOD("set_game_phase", "value"), &GameStateNative::set_game_phase);
    ClassDB::bind_method(D_METHOD("get_game_phase"), &GameStateNative::get_game_phase);

    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "Phase"), "", "get_phase");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "MENU"), "", "get_menu_phase");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "BETWEEN_WAVE"), "", "get_between_wave_phase");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "WAVE_ACTIVE"), "", "get_wave_active_phase");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "PAUSED"), "", "get_paused_phase");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "GAME_OVER"), "", "get_game_over_phase");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "VICTORY"), "", "get_victory_phase");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "luminosity"), "set_luminosity", "get_luminosity");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "sol_credits"), "set_sol_credits", "get_sol_credits");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "current_wave"), "set_current_wave", "get_current_wave");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "flare_charge"), "set_flare_charge", "get_flare_charge");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "waves_since_last_flare"), "set_waves_since_last_flare", "get_waves_since_last_flare");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "performance_score"), "set_performance_score", "get_performance_score");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "enemies_killed_total"), "set_enemies_killed_total", "get_enemies_killed_total");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "waves_cleared"), "set_waves_cleared", "get_waves_cleared");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "burrowers_active"), "set_burrowers_active", "get_burrowers_active");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "music_enabled"), "", "get_music_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "music_volume"), "", "get_music_volume");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "brightness"), "set_brightness", "get_brightness");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "tutorial_completed"), "", "get_tutorial_completed");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "screen_shake_enabled"), "", "get_screen_shake_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_start_waves_enabled"), "", "get_auto_start_waves_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "tech_effects_enabled"), "set_tech_effects_enabled", "get_tech_effects_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "tech_xp"), "", "get_tech_xp");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "unlocked_tech"), "", "get_unlocked_tech");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "test_unlimited_sol_enabled"), "", "get_test_unlimited_sol_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "music_changed_by_user_this_session"), "", "get_music_changed_by_user_this_session");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "game_phase"), "set_game_phase", "get_game_phase");

    ADD_SIGNAL(MethodInfo("luminosity_changed", PropertyInfo(Variant::FLOAT, "new_value")));
    ADD_SIGNAL(MethodInfo("credits_changed", PropertyInfo(Variant::INT, "new_value")));
    ADD_SIGNAL(MethodInfo("score_changed", PropertyInfo(Variant::INT, "new_value")));
    ADD_SIGNAL(MethodInfo("flare_charged"));
    ADD_SIGNAL(MethodInfo("flare_used"));
    ADD_SIGNAL(MethodInfo("game_over_triggered", PropertyInfo(Variant::FLOAT, "final_luminosity"), PropertyInfo(Variant::INT, "killing_wave")));
    ADD_SIGNAL(MethodInfo("victory_triggered", PropertyInfo(Variant::FLOAT, "final_luminosity"), PropertyInfo(Variant::STRING, "rank")));
    ADD_SIGNAL(MethodInfo("burrower_count_changed", PropertyInfo(Variant::INT, "count")));
    ADD_SIGNAL(MethodInfo("phase_changed", PropertyInfo(Variant::INT, "new_phase")));
    ADD_SIGNAL(MethodInfo("music_settings_changed", PropertyInfo(Variant::BOOL, "enabled"), PropertyInfo(Variant::FLOAT, "volume")));
    ADD_SIGNAL(MethodInfo("display_settings_changed", PropertyInfo(Variant::FLOAT, "brightness")));
    ADD_SIGNAL(MethodInfo("tutorial_settings_changed", PropertyInfo(Variant::BOOL, "completed")));
    ADD_SIGNAL(MethodInfo("game_feel_settings_changed", PropertyInfo(Variant::BOOL, "screen_shake_enabled")));
    ADD_SIGNAL(MethodInfo("auto_start_settings_changed", PropertyInfo(Variant::BOOL, "enabled")));
    ADD_SIGNAL(MethodInfo("tech_progress_changed", PropertyInfo(Variant::INT, "tech_xp"), PropertyInfo(Variant::ARRAY, "unlocked_tech")));
    ADD_SIGNAL(MethodInfo("achievement_progress_changed", PropertyInfo(Variant::ARRAY, "new_achievements")));

    BIND_ENUM_CONSTANT(MENU);
    BIND_ENUM_CONSTANT(BETWEEN_WAVE);
    BIND_ENUM_CONSTANT(WAVE_ACTIVE);
    BIND_ENUM_CONSTANT(PAUSED);
    BIND_ENUM_CONSTANT(GAME_OVER);
    BIND_ENUM_CONSTANT(VICTORY);
}

void GameStateNative::_ready() {
    reset_state();
    ensure_brightness_overlay();
    load_audio_settings();
}

void GameStateNative::reset_state() {
    luminosity = 1.0;
    sol_credits = test_unlimited_sol_enabled ? 999999 : 75;
    current_wave = 0;
    flare_charge = 0;
    waves_since_last_flare = 0;
    performance_score = 0;
    enemies_killed_total = 0;
    waves_cleared = 0;
    burrowers_active = 0;
    game_phase = MENU;
}

void GameStateNative::load_audio_settings() {
    Ref<ConfigFile> config;
    config.instantiate();
    const Error error = config->load(SETTINGS_PATH);
    if (error == OK) {
        music_enabled = bool(config->get_value("audio", "music_enabled", music_enabled));
        music_volume = Math::clamp(double(config->get_value("audio", "music_volume", music_volume)), 0.0, 1.0);
        brightness = Math::clamp(double(config->get_value("display", "brightness", brightness)), 0.5, 1.25);
        tutorial_completed = bool(config->get_value("tutorial", "completed", tutorial_completed));
        screen_shake_enabled = bool(config->get_value("gameplay", "screen_shake_enabled", screen_shake_enabled));
        auto_start_waves_enabled = bool(config->get_value("gameplay", "auto_start_waves_enabled", auto_start_waves_enabled));
        tech_effects_enabled = bool(config->get_value("gameplay", "tech_effects_enabled", tech_effects_enabled));
        const int loaded_tech_xp = int(config->get_value("tech", "xp", tech_xp));
        tech_xp = MAX(0, loaded_tech_xp);
        Variant saved_unlocked = config->get_value("tech", "unlocked", unlocked_tech);
        if (saved_unlocked.get_type() == Variant::ARRAY) {
            unlocked_tech = Array(saved_unlocked);
        }
        best_campaign_score = int(config->get_value("records", "campaign_score", best_campaign_score));
        best_campaign_luminosity = int(config->get_value("records", "campaign_luminosity", best_campaign_luminosity));
        best_campaign_rank = String(config->get_value("records", "campaign_rank", best_campaign_rank));
        best_no_flare_score = int(config->get_value("records", "no_flare_score", best_no_flare_score));
        best_no_flare_luminosity = int(config->get_value("records", "no_flare_luminosity", best_no_flare_luminosity));
        best_no_flare_rank = String(config->get_value("records", "no_flare_rank", best_no_flare_rank));
        best_endless_waves = int(config->get_value("records", "endless_waves", best_endless_waves));
        best_endless_score = int(config->get_value("records", "endless_score", best_endless_score));
        best_boss_rush_waves = int(config->get_value("records", "boss_rush_waves", best_boss_rush_waves));
        best_boss_rush_score = int(config->get_value("records", "boss_rush_score", best_boss_rush_score));
        best_daily_seed_waves = int(config->get_value("records", "daily_seed_waves", best_daily_seed_waves));
        best_daily_seed_score = int(config->get_value("records", "daily_seed_score", best_daily_seed_score));
        best_draft_defense_waves = int(config->get_value("records", "draft_defense_waves", best_draft_defense_waves));
        best_draft_defense_score = int(config->get_value("records", "draft_defense_score", best_draft_defense_score));
        Variant saved_achievements = config->get_value("achievements", "unlocked", unlocked_achievements);
        if (saved_achievements.get_type() == Variant::ARRAY) {
            unlocked_achievements = Array(saved_achievements);
        }
        achievement_total_waves = MAX(0, int(config->get_value("achievements", "total_waves", achievement_total_waves)));
        achievement_total_kills = MAX(0, int(config->get_value("achievements", "total_kills", achievement_total_kills)));
        achievement_total_score = MAX(0, int(config->get_value("achievements", "total_score", achievement_total_score)));
        achievement_best_combo = MAX(0, int(config->get_value("achievements", "best_combo", achievement_best_combo)));
        achievement_best_perfect_orbits = MAX(0, int(config->get_value("achievements", "best_perfect_orbits", achievement_best_perfect_orbits)));
        achievement_best_campaign_victory_luminosity = Math::clamp(int(config->get_value("achievements", "best_campaign_victory_luminosity", achievement_best_campaign_victory_luminosity)), 0, 100);
        achievement_lowest_victory_luminosity = Math::clamp(int(config->get_value("achievements", "lowest_victory_luminosity", achievement_lowest_victory_luminosity)), 1, 101);
        Variant saved_mode_victories = config->get_value("achievements", "mode_victories", achievement_mode_victories);
        if (saved_mode_victories.get_type() == Variant::DICTIONARY) achievement_mode_victories = Dictionary(saved_mode_victories);
        Variant saved_tiers = config->get_value("achievements", "tiers", achievement_tiers);
        if (saved_tiers.get_type() == Variant::DICTIONARY) achievement_tiers = Dictionary(saved_tiers);
        const int legacy_no_flare_waves = MAX(0, int(config->get_value("achievements", "best_no_flare_waves", 0)));

        bool migrated_achievements = false;
        const int saved_best_score = MAX(best_campaign_score, MAX(best_no_flare_score, MAX(best_endless_score, MAX(best_boss_rush_score, MAX(best_daily_seed_score, best_draft_defense_score)))));
        if (saved_best_score > achievement_total_score) {
            achievement_total_score = saved_best_score;
            migrated_achievements = true;
        }
        const auto unlock_saved = [&](const String& id, bool earned) {
            if (earned && !has_achievement(id)) {
                unlocked_achievements.append(id);
                migrated_achievements = true;
            }
        };
        unlock_saved("no_dimmers", best_campaign_luminosity >= 100);
        unlock_saved("prime_time_over", best_boss_rush_waves >= 4);
        unlock_saved("same_sun", best_daily_seed_waves >= 6);
        unlock_saved("drafted_greatness", best_draft_defense_waves >= 6);
        unlock_saved("one_more_orbit", best_endless_waves >= 20);
        unlock_saved("flareless_future", legacy_no_flare_waves >= 12);

        const auto migrate_mode_victory = [&](const String& mode, const String& id) {
            if (has_achievement(id) && int(achievement_mode_victories.get(mode, 0)) < 1) {
                achievement_mode_victories[mode] = 1;
                migrated_achievements = true;
            }
        };
        migrate_mode_victory("no_flare", "flareless_future");
        migrate_mode_victory("boss_rush", "prime_time_over");
        migrate_mode_victory("daily_seed", "same_sun");
        migrate_mode_victory("draft_defense", "drafted_greatness");
        if (has_achievement("no_dimmers") && achievement_best_campaign_victory_luminosity < 100) {
            achievement_best_campaign_victory_luminosity = 100;
            migrated_achievements = true;
        }
        if (has_achievement("last_photon") && achievement_lowest_victory_luminosity > 20) {
            achievement_lowest_victory_luminosity = 20;
            migrated_achievements = true;
        }
        for (const AchievementInfo& info : ACHIEVEMENTS) {
            const String id = info.id;
            const int saved_tier = Math::clamp(int(achievement_tiers.get(id, 0)), 0, 3);
            const int migrated_tier = MAX(has_achievement(id) ? 1 : 0, achievement_earned_tier(id));
            if (migrated_tier > saved_tier) {
                achievement_tiers[id] = migrated_tier;
                migrated_achievements = true;
            }
        }
        if (migrated_achievements) save_achievements();
    }
    apply_brightness_overlay();
    emit_signal("music_settings_changed", music_enabled, music_volume);
    emit_signal("display_settings_changed", brightness);
    emit_signal("tutorial_settings_changed", tutorial_completed);
    emit_signal("game_feel_settings_changed", screen_shake_enabled);
    emit_signal("auto_start_settings_changed", auto_start_waves_enabled);
    if (tech_xp != int(config->get_value("tech", "xp", tech_xp))) {
        config->set_value("tech", "xp", tech_xp);
        config->save(SETTINGS_PATH);
    }
    emit_signal("tech_progress_changed", tech_xp, unlocked_tech);
}

void GameStateNative::save_audio_settings() {
    Ref<ConfigFile> config = settings_config();
    config->set_value("audio", "music_enabled", music_enabled);
    config->set_value("audio", "music_volume", music_volume);
    config->save(SETTINGS_PATH);
}

void GameStateNative::ensure_music_audible() {
    if (music_changed_by_user_this_session) {
        return;
    }
    if (music_enabled && music_volume >= MIN_AUDIBLE_MUSIC_VOLUME) {
        return;
    }
    music_enabled = true;
    music_volume = MAX(music_volume, DEFAULT_MUSIC_VOLUME);
    save_audio_settings();
    emit_signal("music_settings_changed", music_enabled, music_volume);
}

void GameStateNative::set_music_enabled(bool enabled) {
    music_changed_by_user_this_session = true;
    music_enabled = enabled;
    save_audio_settings();
    emit_signal("music_settings_changed", music_enabled, music_volume);
}

void GameStateNative::set_music_volume(double volume) {
    music_changed_by_user_this_session = true;
    music_volume = Math::clamp(volume, 0.0, 1.0);
    save_audio_settings();
    emit_signal("music_settings_changed", music_enabled, music_volume);
}

double GameStateNative::get_music_volume_db() const {
    if (!music_enabled || music_volume <= 0.0) {
        return -80.0;
    }
    return UtilityFunctions::linear_to_db(music_volume);
}

void GameStateNative::set_brightness(double value) {
    brightness = Math::clamp(value, 0.5, 1.25);
    save_display_settings();
    apply_brightness_overlay();
    emit_signal("display_settings_changed", brightness);
}

void GameStateNative::set_tutorial_completed(bool completed) {
    tutorial_completed = completed;
    Ref<ConfigFile> config = settings_config();
    config->set_value("tutorial", "completed", tutorial_completed);
    config->save(SETTINGS_PATH);
    emit_signal("tutorial_settings_changed", tutorial_completed);
}

void GameStateNative::set_screen_shake_enabled(bool enabled) {
    screen_shake_enabled = enabled;
    Ref<ConfigFile> config = settings_config();
    config->set_value("gameplay", "screen_shake_enabled", screen_shake_enabled);
    config->save(SETTINGS_PATH);
    emit_signal("game_feel_settings_changed", screen_shake_enabled);
}

void GameStateNative::set_auto_start_waves_enabled(bool enabled) {
    auto_start_waves_enabled = enabled;
    Ref<ConfigFile> config = settings_config();
    config->set_value("gameplay", "auto_start_waves_enabled", auto_start_waves_enabled);
    config->save(SETTINGS_PATH);
    emit_signal("auto_start_settings_changed", auto_start_waves_enabled);
}

void GameStateNative::set_tech_effects_enabled(bool enabled) {
    tech_effects_enabled = enabled;
    Ref<ConfigFile> config = settings_config();
    config->set_value("gameplay", "tech_effects_enabled", tech_effects_enabled);
    config->save(SETTINGS_PATH);
}

int GameStateNative::add_tech_xp(int amount) {
    if (amount <= 0) {
        tech_xp = MAX(0, tech_xp);
        return tech_xp;
    }
    tech_xp = MAX(0, tech_xp) + amount;
    save_tech_progress();
    emit_signal("tech_progress_changed", tech_xp, unlocked_tech);
    return tech_xp;
}

bool GameStateNative::unlock_tech(const String& tech_id, int cost, const Array& requirements) {
    if (tech_id.is_empty()) {
        return false;
    }
    if (has_tech(tech_id)) {
        return true;
    }
    if (tech_id == "apex_master") {
        return false;
    }
    if (is_apex_tech(tech_id) && has_any_apex_tech(unlocked_tech)) {
        return false;
    }
    for (int i = 0; i < requirements.size(); ++i) {
        if (!has_tech(String(requirements[i]))) {
            return false;
        }
    }
    tech_xp = MAX(0, tech_xp);
    const int safe_cost = MAX(cost, 0);
    if (tech_xp < safe_cost) {
        return false;
    }
    tech_xp -= safe_cost;
    unlocked_tech.append(tech_id);
    save_tech_progress();
    emit_signal("tech_progress_changed", tech_xp, unlocked_tech);
    return true;
}

bool GameStateNative::has_tech(const String& tech_id) const {
    if (is_apex_tech(tech_id)) {
        for (int i = 0; i < unlocked_tech.size(); ++i) {
            const String unlocked_id = String(unlocked_tech[i]);
            if (is_apex_tech(unlocked_id)) {
                return unlocked_id == tech_id;
            }
        }
        return false;
    }
    for (int i = 0; i < unlocked_tech.size(); ++i) {
        if (String(unlocked_tech[i]) == tech_id) {
            return true;
        }
    }
    return false;
}

Dictionary GameStateNative::record_run(const String& run_mode, int score, int waves, int luminosity_percent, const String& rank, bool victory, int perfect_orbits, int best_combo) {
    String mode = run_mode;
    if (mode != "endless" && mode != "no_flare" && mode != "boss_rush" && mode != "daily_seed" && mode != "draft_defense") {
        mode = "campaign";
    }
    bool new_score = false;
    bool new_luminosity = false;
    bool new_waves = false;
    auto update_wave_score_record = [&](int& best_waves, int& best_score) {
        new_waves = waves > best_waves;
        new_score = score > best_score;
        if (new_waves) best_waves = waves;
        if (new_score) best_score = score;
    };

    if (mode == "endless") {
        update_wave_score_record(best_endless_waves, best_endless_score);
    } else if (mode == "boss_rush") {
        update_wave_score_record(best_boss_rush_waves, best_boss_rush_score);
    } else if (mode == "daily_seed") {
        update_wave_score_record(best_daily_seed_waves, best_daily_seed_score);
    } else if (mode == "draft_defense") {
        update_wave_score_record(best_draft_defense_waves, best_draft_defense_score);
    } else if (mode == "no_flare") {
        new_score = score > best_no_flare_score;
        new_luminosity = luminosity_percent > best_no_flare_luminosity || best_no_flare_rank.is_empty();
        if (new_score) best_no_flare_score = score;
        if (new_luminosity) {
            best_no_flare_luminosity = luminosity_percent;
            best_no_flare_rank = rank;
        }
    } else {
        new_score = score > best_campaign_score;
        new_luminosity = luminosity_percent > best_campaign_luminosity || best_campaign_rank.is_empty();
        if (new_score) best_campaign_score = score;
        if (new_luminosity) {
            best_campaign_luminosity = luminosity_percent;
            best_campaign_rank = rank;
        }
    }

    if (new_score || new_luminosity || new_waves) {
        save_records();
    }

    Dictionary summary = best_run_summary(mode);
    summary["new_score"] = new_score;
    summary["new_luminosity"] = new_luminosity;
    summary["new_waves"] = new_waves;
    summary["new_best"] = new_score || new_luminosity || new_waves;
    summary["new_achievements"] = update_achievements(mode, score, waves, luminosity_percent, victory, perfect_orbits, best_combo);
    return summary;
}

int GameStateNative::achievement_progress_value(const String& achievement_id) const {
    for (const AchievementInfo& info : ACHIEVEMENTS) {
        if (achievement_id != info.id) continue;
        switch (info.rule) {
            case TOTAL_WAVES: return achievement_total_waves;
            case TOTAL_KILLS: return achievement_total_kills;
            case TOTAL_SCORE: return achievement_total_score;
            case BEST_COMBO: return achievement_best_combo;
            case BEST_PERFECT_ORBITS: return achievement_best_perfect_orbits;
            case CAMPAIGN_FULL_SHINE: return achievement_best_campaign_victory_luminosity;
            case LAST_LIGHT_VICTORY: return achievement_lowest_victory_luminosity;
            case MODE_VICTORY: return MAX(0, int(achievement_mode_victories.get(String(info.mode), 0)));
            case ENDLESS_WAVES: return best_endless_waves;
        }
    }
    return 0;
}

int GameStateNative::achievement_earned_tier(const String& achievement_id) const {
    for (const AchievementInfo& info : ACHIEVEMENTS) {
        if (achievement_id != info.id) continue;
        const int current = achievement_progress_value(achievement_id);
        if (info.lower_is_better) {
            if (current <= info.gold_target) return 3;
            if (current <= info.silver_target) return 2;
            if (current <= info.bronze_target) return 1;
        } else {
            if (current >= info.gold_target) return 3;
            if (current >= info.silver_target) return 2;
            if (current >= info.bronze_target) return 1;
        }
        return 0;
    }
    return 0;
}

Array GameStateNative::achievement_board() const {
    static const char* TIER_NAMES[] = {"UNRANKED", "BRONZE", "SILVER", "GOLD"};
    static const Color TIER_COLORS[] = {
        Color(0.30, 0.42, 0.52),
        Color(0.80, 0.45, 0.20),
        Color(0.76, 0.84, 0.92),
        Color(1.0, 0.78, 0.22),
    };

    Array board;
    for (const AchievementInfo& info : ACHIEVEMENTS) {
        const String id = info.id;
        const int current = achievement_progress_value(id);
        const int tier = MAX(Math::clamp(int(achievement_tiers.get(id, 0)), 0, 3), achievement_earned_tier(id));
        const int target = tier == 0 ? info.bronze_target : (tier == 1 ? info.silver_target : info.gold_target);
        double progress = 1.0;
        if (tier < 3) {
            progress = info.lower_is_better
                ? (current <= 100 ? Math::clamp(double(target) / MAX(1, current), 0.0, 1.0) : 0.0)
                : Math::clamp(double(current) / MAX(1, target), 0.0, 1.0);
        }

        String progress_text;
        if (tier >= 3) {
            progress_text = "GOLD STANDARD SECURED";
        } else if (info.lower_is_better) {
            progress_text = current <= 100
                ? vformat("BEST WIN  %d%%  //  NEXT %s <= %d%%", current, TIER_NAMES[tier + 1], target)
                : vformat("NEXT %s  WIN AT <= %d%% LIGHT", TIER_NAMES[tier + 1], target);
        } else if (info.rule == CAMPAIGN_FULL_SHINE) {
            progress_text = vformat("NEXT %s  %d%% / %d%% LIGHT", TIER_NAMES[tier + 1], current, target);
        } else {
            progress_text = vformat("NEXT %s  %d / %d %s", TIER_NAMES[tier + 1], current, target, info.unit);
        }

        Dictionary item;
        item["id"] = id;
        item["title"] = info.title;
        item["description"] = info.description;
        item["unlocked"] = tier > 0;
        item["tier_level"] = tier;
        item["tier"] = TIER_NAMES[tier];
        item["medal_color"] = TIER_COLORS[tier];
        item["current"] = current;
        item["target"] = target;
        item["progress"] = progress;
        item["progress_text"] = progress_text;
        item["icon_index"] = info.icon_index;
        item["accent"] = info.accent;
        board.append(item);
    }
    return board;
}

Array GameStateNative::update_achievements(const String& mode, int score, int waves, int luminosity_percent, bool victory, int perfect_orbits, int best_combo) {
    Array newly_unlocked;
    if (test_unlimited_sol_enabled) {
        return newly_unlocked;
    }
    achievement_total_waves += MAX(0, waves);
    achievement_total_kills += MAX(0, enemies_killed_total);
    achievement_total_score += MAX(0, score);
    achievement_best_combo = MAX(achievement_best_combo, best_combo);
    achievement_best_perfect_orbits = MAX(achievement_best_perfect_orbits, perfect_orbits);
    if (victory && luminosity_percent > 0) {
        achievement_lowest_victory_luminosity = MIN(achievement_lowest_victory_luminosity, luminosity_percent);
        if (mode == "campaign") achievement_best_campaign_victory_luminosity = MAX(achievement_best_campaign_victory_luminosity, luminosity_percent);
        if (mode == "no_flare" || mode == "boss_rush" || mode == "daily_seed" || mode == "draft_defense") {
            achievement_mode_victories[mode] = MAX(0, int(achievement_mode_victories.get(mode, 0))) + 1;
        }
    }

    static const char* TIER_NAMES[] = {"UNRANKED", "BRONZE", "SILVER", "GOLD"};
    for (const AchievementInfo& info : ACHIEVEMENTS) {
        const String id = info.id;
        const int previous_tier = Math::clamp(int(achievement_tiers.get(id, 0)), 0, 3);
        const int earned_tier = achievement_earned_tier(id);
        if (earned_tier > previous_tier) {
            const bool was_unlocked = has_achievement(id);
            achievement_tiers[id] = earned_tier;
            if (!was_unlocked) unlocked_achievements.append(id);
            newly_unlocked.append(vformat("%s MEDAL  -  %s", TIER_NAMES[earned_tier], info.title));
        }
    }
    save_achievements();
    if (!newly_unlocked.is_empty()) emit_signal("achievement_progress_changed", newly_unlocked);
    return newly_unlocked;
}

bool GameStateNative::has_achievement(const String& achievement_id) const {
    if (int(achievement_tiers.get(achievement_id, 0)) > 0) return true;
    for (int i = 0; i < unlocked_achievements.size(); ++i) {
        if (String(unlocked_achievements[i]) == achievement_id) return true;
    }
    return false;
}

Dictionary GameStateNative::best_run_summary(const String& run_mode) const {
    String mode = run_mode;
    if (mode != "endless" && mode != "no_flare" && mode != "boss_rush" && mode != "daily_seed" && mode != "draft_defense") {
        mode = "campaign";
    }
    Dictionary summary;
    summary["mode"] = mode;
    if (mode == "endless") {
        summary["waves"] = best_endless_waves;
        summary["score"] = best_endless_score;
        return summary;
    } else if (mode == "boss_rush") {
        summary["waves"] = best_boss_rush_waves;
        summary["score"] = best_boss_rush_score;
        return summary;
    } else if (mode == "daily_seed") {
        summary["waves"] = best_daily_seed_waves;
        summary["score"] = best_daily_seed_score;
        return summary;
    } else if (mode == "draft_defense") {
        summary["waves"] = best_draft_defense_waves;
        summary["score"] = best_draft_defense_score;
        return summary;
    }

    const bool no_flare = mode == "no_flare";
    const String rank = no_flare ? best_no_flare_rank : best_campaign_rank;
    summary["score"] = no_flare ? best_no_flare_score : best_campaign_score;
    summary["luminosity"] = no_flare ? best_no_flare_luminosity : best_campaign_luminosity;
    summary["rank"] = rank.is_empty() ? String("UNRANKED") : rank;
    return summary;
}

void GameStateNative::enable_test_run(int start_wave) {
    test_unlimited_sol_enabled = true;
    pending_test_start_wave = Math::clamp(start_wave, 1, 12);
}

void GameStateNative::clear_test_run() {
    test_unlimited_sol_enabled = false;
    pending_test_start_wave = 0;
}

int GameStateNative::consume_test_start_wave() {
    const int start_wave = pending_test_start_wave;
    pending_test_start_wave = 0;
    return start_wave;
}

void GameStateNative::damage_sun(double amount) {
    if (game_phase == GAME_OVER) {
        return;
    }
    luminosity = Math::clamp(luminosity - amount, 0.0, 1.0);
    emit_signal("luminosity_changed", luminosity);
    if (luminosity <= 0.0) {
        trigger_game_over();
    }
}

int GameStateNative::get_luminosity_percent() const {
    return int(luminosity * 100.0);
}

void GameStateNative::add_credits(int amount) {
    sol_credits += amount;
    emit_signal("credits_changed", sol_credits);
}

bool GameStateNative::spend_credits(int amount) {
    if (test_unlimited_sol_enabled) {
        emit_signal("credits_changed", sol_credits);
        return true;
    }
    if (sol_credits < amount) {
        return false;
    }
    sol_credits -= amount;
    emit_signal("credits_changed", sol_credits);
    return true;
}

bool GameStateNative::can_afford(int amount) const {
    if (test_unlimited_sol_enabled) {
        return true;
    }
    return sol_credits >= amount;
}

int GameStateNative::get_tower_cost(const String& tower_type) const {
    return int(tower_costs().get(tower_type, 30));
}

int GameStateNative::get_upgrade_cost(const String& tower_type) const {
    return int(tower_upgrade_costs().get(tower_type, 50));
}

void GameStateNative::add_score(int amount) {
    performance_score += amount;
    emit_signal("score_changed", performance_score);
}

void GameStateNative::on_enemy_killed(int variant_id) {
    enemies_killed_total += 1;
    const int values[] = {10, 20, 40, 30, 25, 200};
    const int index = Math::clamp(variant_id, 0, 5);
    add_score(values[index]);
}

void GameStateNative::on_wave_cleared() {
    waves_cleared += 1;
    waves_since_last_flare += 1;
    const int flare_wave_count = (tech_effects_enabled && has_tech("flare_battery")) ? 2 : 3;
    if (waves_since_last_flare >= flare_wave_count && flare_charge == 0) {
        flare_charge = 1;
        waves_since_last_flare = 0;
        emit_signal("flare_charged");
    }
}

bool GameStateNative::try_trigger_flare() {
    if (test_unlimited_sol_enabled) {
        emit_signal("flare_used");
        return true;
    }
    if (flare_charge <= 0) {
        return false;
    }
    flare_charge -= 1;
    emit_signal("flare_used");
    return true;
}

void GameStateNative::add_burrower() {
    burrowers_active += 1;
    emit_signal("burrower_count_changed", burrowers_active);
}

void GameStateNative::remove_burrower() {
    burrowers_active = MAX(0, burrowers_active - 1);
    emit_signal("burrower_count_changed", burrowers_active);
}

void GameStateNative::set_phase(int new_phase) {
    game_phase = new_phase;
    emit_signal("phase_changed", game_phase);
}

String GameStateNative::get_rank() const {
    if (luminosity > 0.8) return "FULL SHINE";
    if (luminosity > 0.6) return "BRIGHT";
    if (luminosity > 0.2) return "DIM BUT ALIVE";
    return "LAST LIGHT";
}

void GameStateNative::trigger_victory() {
    game_phase = VICTORY;
    emit_signal("victory_triggered", luminosity, get_rank());
}

Dictionary GameStateNative::get_phase() const {
    Dictionary phase;
    phase["MENU"] = MENU;
    phase["BETWEEN_WAVE"] = BETWEEN_WAVE;
    phase["WAVE_ACTIVE"] = WAVE_ACTIVE;
    phase["PAUSED"] = PAUSED;
    phase["GAME_OVER"] = GAME_OVER;
    phase["VICTORY"] = VICTORY;
    return phase;
}

int GameStateNative::get_menu_phase() const { return MENU; }
int GameStateNative::get_between_wave_phase() const { return BETWEEN_WAVE; }
int GameStateNative::get_wave_active_phase() const { return WAVE_ACTIVE; }
int GameStateNative::get_paused_phase() const { return PAUSED; }
int GameStateNative::get_game_over_phase() const { return GAME_OVER; }
int GameStateNative::get_victory_phase() const { return VICTORY; }

double GameStateNative::get_luminosity() const { return luminosity; }
void GameStateNative::set_luminosity(double value) { luminosity = value; }
int GameStateNative::get_sol_credits() const { return sol_credits; }
void GameStateNative::set_sol_credits(int value) { sol_credits = value; }
int GameStateNative::get_current_wave() const { return current_wave; }
void GameStateNative::set_current_wave(int value) { current_wave = value; }
int GameStateNative::get_flare_charge() const { return test_unlimited_sol_enabled ? 1 : flare_charge; }
void GameStateNative::set_flare_charge(int value) { flare_charge = value; }
int GameStateNative::get_waves_since_last_flare() const { return waves_since_last_flare; }
void GameStateNative::set_waves_since_last_flare(int value) { waves_since_last_flare = value; }
int GameStateNative::get_performance_score() const { return performance_score; }
void GameStateNative::set_performance_score(int value) { performance_score = value; }
int GameStateNative::get_enemies_killed_total() const { return enemies_killed_total; }
void GameStateNative::set_enemies_killed_total(int value) { enemies_killed_total = value; }
int GameStateNative::get_waves_cleared() const { return waves_cleared; }
void GameStateNative::set_waves_cleared(int value) { waves_cleared = value; }
int GameStateNative::get_burrowers_active() const { return burrowers_active; }
void GameStateNative::set_burrowers_active(int value) { burrowers_active = value; }
bool GameStateNative::get_music_enabled() const { return music_enabled; }
double GameStateNative::get_music_volume() const { return music_volume; }
double GameStateNative::get_brightness() const { return brightness; }
bool GameStateNative::get_tutorial_completed() const { return tutorial_completed; }
bool GameStateNative::get_screen_shake_enabled() const { return screen_shake_enabled; }
bool GameStateNative::get_auto_start_waves_enabled() const { return auto_start_waves_enabled; }
bool GameStateNative::get_tech_effects_enabled() const { return tech_effects_enabled; }
int GameStateNative::get_tech_xp() const { return MAX(0, tech_xp); }
Array GameStateNative::get_unlocked_tech() const { return unlocked_tech; }
bool GameStateNative::get_test_unlimited_sol_enabled() const { return test_unlimited_sol_enabled; }
bool GameStateNative::get_music_changed_by_user_this_session() const { return music_changed_by_user_this_session; }
int GameStateNative::get_game_phase() const { return game_phase; }
void GameStateNative::set_game_phase(int value) { game_phase = value; }

Ref<ConfigFile> GameStateNative::settings_config() const {
    Ref<ConfigFile> config;
    config.instantiate();
    config->load(SETTINGS_PATH);
    return config;
}

void GameStateNative::ensure_brightness_overlay() {
    if (brightness_layer != nullptr && brightness_dimmer != nullptr) {
        return;
    }

    brightness_layer = memnew(CanvasLayer);
    brightness_layer->set_name("BrightnessLayer");
    brightness_layer->set_layer(128);
    brightness_layer->set_process_mode(Node::PROCESS_MODE_ALWAYS);
    add_child(brightness_layer);

    brightness_dimmer = memnew(ColorRect);
    brightness_dimmer->set_name("BrightnessDimmer");
    brightness_dimmer->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
    brightness_dimmer->set_anchors_preset(Control::PRESET_FULL_RECT);
    brightness_dimmer->set_offset(SIDE_LEFT, 0.0);
    brightness_dimmer->set_offset(SIDE_TOP, 0.0);
    brightness_dimmer->set_offset(SIDE_RIGHT, 0.0);
    brightness_dimmer->set_offset(SIDE_BOTTOM, 0.0);
    brightness_layer->add_child(brightness_dimmer);
    apply_brightness_overlay();
}

void GameStateNative::apply_brightness_overlay() {
    if (brightness_dimmer == nullptr) {
        return;
    }
    // ponytail: overlay brightness, swap for a shader/gamma pass if color accuracy matters.
    if (brightness < 1.0) {
        brightness_dimmer->set_color(Color(0.0, 0.0, 0.0, 1.0 - brightness));
        brightness_dimmer->set_visible(true);
    } else if (brightness > 1.0) {
        brightness_dimmer->set_color(Color(1.0, 1.0, 1.0, (brightness - 1.0) * 0.65));
        brightness_dimmer->set_visible(true);
    } else {
        brightness_dimmer->set_visible(false);
    }
}

void GameStateNative::save_display_settings() {
    Ref<ConfigFile> config = settings_config();
    config->set_value("display", "brightness", brightness);
    config->save(SETTINGS_PATH);
}

void GameStateNative::save_tech_progress() {
    Ref<ConfigFile> config = settings_config();
    tech_xp = MAX(0, tech_xp);
    config->set_value("tech", "xp", tech_xp);
    config->set_value("tech", "unlocked", unlocked_tech);
    config->save(SETTINGS_PATH);
}

void GameStateNative::save_records() {
    Ref<ConfigFile> config = settings_config();
    config->set_value("records", "campaign_score", best_campaign_score);
    config->set_value("records", "campaign_luminosity", best_campaign_luminosity);
    config->set_value("records", "campaign_rank", best_campaign_rank);
    config->set_value("records", "no_flare_score", best_no_flare_score);
    config->set_value("records", "no_flare_luminosity", best_no_flare_luminosity);
    config->set_value("records", "no_flare_rank", best_no_flare_rank);
    config->set_value("records", "endless_waves", best_endless_waves);
    config->set_value("records", "endless_score", best_endless_score);
    config->set_value("records", "boss_rush_waves", best_boss_rush_waves);
    config->set_value("records", "boss_rush_score", best_boss_rush_score);
    config->set_value("records", "daily_seed_waves", best_daily_seed_waves);
    config->set_value("records", "daily_seed_score", best_daily_seed_score);
    config->set_value("records", "draft_defense_waves", best_draft_defense_waves);
    config->set_value("records", "draft_defense_score", best_draft_defense_score);
    config->save(SETTINGS_PATH);
}

void GameStateNative::save_achievements() {
    Ref<ConfigFile> config = settings_config();
    config->set_value("achievements", "unlocked", unlocked_achievements);
    config->set_value("achievements", "total_waves", achievement_total_waves);
    config->set_value("achievements", "total_kills", achievement_total_kills);
    config->set_value("achievements", "total_score", achievement_total_score);
    config->set_value("achievements", "best_combo", achievement_best_combo);
    config->set_value("achievements", "best_perfect_orbits", achievement_best_perfect_orbits);
    config->set_value("achievements", "best_campaign_victory_luminosity", achievement_best_campaign_victory_luminosity);
    config->set_value("achievements", "lowest_victory_luminosity", achievement_lowest_victory_luminosity);
    config->set_value("achievements", "mode_victories", achievement_mode_victories);
    config->set_value("achievements", "tiers", achievement_tiers);
    config->save(SETTINGS_PATH);
}

void GameStateNative::trigger_game_over() {
    game_phase = GAME_OVER;
    emit_signal("game_over_triggered", luminosity, current_wave);
}

Dictionary GameStateNative::tower_costs() const {
    Dictionary costs;
    costs["photon_splitter"] = 22;
    costs["cryo_probe"] = 28;
    costs["bio_lab"] = 40;
    costs["magnetic_net"] = 38;
    costs["helios_cannon"] = 68;
    costs["tardigrade_bomb"] = 58;
    return costs;
}

Dictionary GameStateNative::tower_upgrade_costs() const {
    Dictionary costs;
    costs["photon_splitter"] = 28;
    costs["cryo_probe"] = 36;
    costs["bio_lab"] = 52;
    costs["magnetic_net"] = 48;
    costs["helios_cannon"] = 88;
    costs["tardigrade_bomb"] = 74;
    return costs;
}

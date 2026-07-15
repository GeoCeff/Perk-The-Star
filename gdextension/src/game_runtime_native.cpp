#include "game_runtime_native.h"

#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

using namespace godot;

void GameRuntimeNative::_bind_methods() {
    ClassDB::bind_method(D_METHOD("ease_out_cubic", "value"), &GameRuntimeNative::ease_out_cubic);
    ClassDB::bind_method(D_METHOD("ease_in_out_sine", "value"), &GameRuntimeNative::ease_in_out_sine);
    ClassDB::bind_method(D_METHOD("sun_pos", "viewport_size"), &GameRuntimeNative::sun_pos);
    ClassDB::bind_method(D_METHOD("screen_shake_offset", "timer", "enabled", "strength"), &GameRuntimeNative::screen_shake_offset);
    ClassDB::bind_method(D_METHOD("can_build_towers", "phase", "between_wave", "wave_active"), &GameRuntimeNative::can_build_towers);
    ClassDB::bind_method(D_METHOD("bgm_path_for_wave", "wave_number", "early", "mid", "late", "boss"), &GameRuntimeNative::bgm_path_for_wave);
    ClassDB::bind_method(D_METHOD("run_tech_xp_award", "performance_score", "enemies_killed_total", "waves_cleared", "luminosity_percent", "max_waves", "victory", "run_mode"), &GameRuntimeNative::run_tech_xp_award);
    ClassDB::bind_method(D_METHOD("run_record_text", "summary", "fallback_run_mode", "boss_rush_waves", "daily_seed_waves", "draft_defense_waves"), &GameRuntimeNative::run_record_text);
    ClassDB::bind_method(
        D_METHOD("physics_projectile_hit_index", "enemies", "pos", "previous_pos", "base_hit_radius"),
        &GameRuntimeNative::physics_projectile_hit_index);
    ClassDB::bind_method(D_METHOD("enemy_index_by_uid", "enemies", "enemy_uid"), &GameRuntimeNative::enemy_index_by_uid);
    ClassDB::bind_method(
        D_METHOD("projectile_segment_hits_point", "previous_pos", "pos", "target_pos", "hit_radius"),
        &GameRuntimeNative::projectile_segment_hits_point);
}

double GameRuntimeNative::ease_out_cubic(double value) const {
    const double t = Math::clamp(value, 0.0, 1.0);
    return 1.0 - Math::pow(1.0 - t, 3.0);
}

double GameRuntimeNative::ease_in_out_sine(double value) const {
    const double t = Math::clamp(value, 0.0, 1.0);
    return 0.5 - Math::cos(t * Math_PI) * 0.5;
}

Vector2 GameRuntimeNative::sun_pos(const Vector2& viewport_size) const {
    return viewport_size * 0.5;
}

Vector2 GameRuntimeNative::screen_shake_offset(double timer, bool enabled, double strength) const {
    if (timer <= 0.0 || !enabled) {
        return Vector2();
    }
    const double fade = Math::clamp(timer / 0.34, 0.0, 1.0);
    const double time_seconds = double(Time::get_singleton()->get_ticks_msec()) / 1000.0;
    return Vector2(Math::sin(time_seconds * 73.0), Math::cos(time_seconds * 61.0)) * strength * fade;
}

bool GameRuntimeNative::can_build_towers(int phase, int between_wave, int wave_active) const {
    return phase == between_wave || phase == wave_active;
}

String GameRuntimeNative::bgm_path_for_wave(int wave_number, const String& early, const String& mid, const String& late, const String& boss) const {
    if (wave_number >= 12) return boss;
    if (wave_number >= 9) return late;
    if (wave_number >= 5) return mid;
    return early;
}

Dictionary GameRuntimeNative::run_tech_xp_award(int performance_score, int enemies_killed_total, int waves_cleared, int luminosity_percent, int max_waves, bool victory, const String& run_mode) const {
    const int score_xp = static_cast<int>(std::floor(static_cast<double>(performance_score) / 10.0));
    const int kill_xp = enemies_killed_total * 3;
    const int wave_xp = waves_cleared * 75;
    const int luminosity_bonus = std::max(0, luminosity_percent);
    const int endless_bonus = run_mode == "endless" ? std::max(0, waves_cleared - max_waves) * 35 : 0;
    const int victory_bonus = victory ? 400 : 0;
    const int no_flare_bonus = run_mode == "no_flare" ? waves_cleared * 25 + (victory ? 250 : 0) : 0;
    const int boss_rush_bonus = run_mode == "boss_rush" ? waves_cleared * 60 + (victory ? 300 : 0) : 0;
    const int daily_bonus = run_mode == "daily_seed" ? waves_cleared * 35 + (victory ? 180 : 0) : 0;
    const int draft_bonus = run_mode == "draft_defense" ? waves_cleared * 45 + (victory ? 220 : 0) : 0;
    const int amount = std::max(1, score_xp + kill_xp + wave_xp + luminosity_bonus + endless_bonus + victory_bonus + no_flare_bonus + boss_rush_bonus + daily_bonus + draft_bonus);

    Array xp_parts;
    xp_parts.append(vformat("SCORE %d", score_xp));
    xp_parts.append(vformat("KILLS %d", kill_xp));
    xp_parts.append(vformat("WAVES %d", wave_xp));
    xp_parts.append(vformat("LUM %d", luminosity_bonus));
    if (endless_bonus > 0) xp_parts.append(vformat("ENDLESS %d", endless_bonus));
    if (no_flare_bonus > 0) xp_parts.append(vformat("NO-FLARE %d", no_flare_bonus));
    if (boss_rush_bonus > 0) xp_parts.append(vformat("BOSS RUSH %d", boss_rush_bonus));
    if (daily_bonus > 0) xp_parts.append(vformat("DAILY %d", daily_bonus));
    if (draft_bonus > 0) xp_parts.append(vformat("DRAFT %d", draft_bonus));
    if (victory_bonus > 0) xp_parts.append(vformat("VICTORY %d", victory_bonus));

    Dictionary award;
    award["amount"] = amount;
    award["breakdown"] = vformat("XP: %s = %d", String(" + ").join(xp_parts), amount);
    return award;
}

String GameRuntimeNative::run_record_text(const Dictionary& summary, const String& fallback_run_mode, int boss_rush_waves, int daily_seed_waves, int draft_defense_waves) const {
    if (summary.is_empty()) {
        return "";
    }
    const String label = bool(summary.get("new_best", false)) ? "NEW BEST" : "BEST";
    const String mode = String(summary.get("mode", fallback_run_mode));
    const int waves = static_cast<int>(summary.get("waves", 0));
    const int score = static_cast<int>(summary.get("score", 0));
    const String rank = String(summary.get("rank", "UNRANKED"));
    const int luminosity = static_cast<int>(summary.get("luminosity", 0));
    if (mode == "endless") {
        return waves <= 0 && score <= 0 ? String("") : vformat("%s ENDLESS: WAVE %d | SCORE %d", label, waves, score);
    }
    if (mode == "boss_rush") {
        return waves <= 0 && score <= 0 ? String("") : vformat("%s BOSS RUSH: WAVE %d/%d | SCORE %d", label, waves, boss_rush_waves, score);
    }
    if (mode == "daily_seed") {
        return waves <= 0 && score <= 0 ? String("") : vformat("%s DAILY: WAVE %d/%d | SCORE %d", label, waves, daily_seed_waves, score);
    }
    if (mode == "draft_defense") {
        return waves <= 0 && score <= 0 ? String("") : vformat("%s DRAFT: WAVE %d/%d | SCORE %d", label, waves, draft_defense_waves, score);
    }
    if (mode == "no_flare") {
        return vformat("%s NO-FLARE: SCORE %d | %s | LUM %d%%", label, score, rank, luminosity);
    }
    return vformat("%s CAMPAIGN: SCORE %d | %s | LUM %d%%", label, score, rank, luminosity);
}

int GameRuntimeNative::physics_projectile_hit_index(const Array& enemies, const Vector2& pos, const Vector2& previous_pos, double base_hit_radius) const {
    int best_index = -1;
    double best_dist_squared = std::numeric_limits<double>::infinity();
    const Vector2 segment = pos - previous_pos;
    const bool has_segment = segment.length_squared() > 0.001f;

    for (int i = 0; i < enemies.size(); ++i) {
        if (enemies[i].get_type() != Variant::DICTIONARY) {
            continue;
        }
        const Dictionary enemy = enemies[i];
        const double radius = static_cast<double>(enemy.get("radius", base_hit_radius));
        const double hit_radius = MAX(base_hit_radius, radius * 0.95);
        const Vector2 enemy_pos = enemy.get("pos", Vector2());
        Vector2 closest_pos = pos;
        if (has_segment) {
            const double t = Math::clamp(
                static_cast<double>((enemy_pos - previous_pos).dot(segment)) /
                    static_cast<double>(segment.length_squared()),
                0.0,
                1.0);
            closest_pos = previous_pos + segment * static_cast<float>(t);
        }
        const double dist_squared = static_cast<double>(closest_pos.distance_squared_to(enemy_pos));
        if (dist_squared <= hit_radius * hit_radius && dist_squared < best_dist_squared) {
            best_dist_squared = dist_squared;
            best_index = i;
        }
    }
    return best_index;
}

int GameRuntimeNative::enemy_index_by_uid(const Array& enemies, int enemy_uid) const {
    if (enemy_uid < 0) {
        return -1;
    }
    for (int i = 0; i < enemies.size(); ++i) {
        if (enemies[i].get_type() != Variant::DICTIONARY) {
            continue;
        }
        const Dictionary enemy = enemies[i];
        if (static_cast<int>(enemy.get("uid", -1)) == enemy_uid) {
            return i;
        }
    }
    return -1;
}

bool GameRuntimeNative::projectile_segment_hits_point(const Vector2& previous_pos, const Vector2& pos, const Vector2& target_pos, double hit_radius) const {
    const Vector2 segment = pos - previous_pos;
    if (segment.length_squared() <= 0.001f) {
        return pos.distance_to(target_pos) <= hit_radius;
    }
    const double t = Math::clamp(
        static_cast<double>((target_pos - previous_pos).dot(segment)) /
            static_cast<double>(segment.length_squared()),
        0.0,
        1.0);
    const Vector2 closest_pos = previous_pos + segment * static_cast<float>(t);
    return closest_pos.distance_to(target_pos) <= hit_radius;
}

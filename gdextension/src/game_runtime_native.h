#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace godot {

class GameRuntimeNative : public RefCounted {
    GDCLASS(GameRuntimeNative, RefCounted)

protected:
    static void _bind_methods();

public:
    double ease_out_cubic(double value) const;
    double ease_in_out_sine(double value) const;
    Vector2 sun_pos(const Vector2& viewport_size) const;
    Vector2 screen_shake_offset(double timer, bool enabled, double strength) const;
    bool can_build_towers(int phase, int between_wave, int wave_active) const;
    String bgm_path_for_wave(int wave_number, const String& early, const String& mid, const String& late, const String& boss) const;
    Dictionary run_tech_xp_award(int performance_score, int enemies_killed_total, int waves_cleared, int luminosity_percent, int max_waves, bool victory, const String& run_mode) const;
    String run_record_text(const Dictionary& summary, const String& fallback_run_mode, int boss_rush_waves, int daily_seed_waves, int draft_defense_waves) const;
    Dictionary end_state_view_data(const Dictionary& state) const;
    int physics_projectile_hit_index(const Array& enemies, const Vector2& pos, const Vector2& previous_pos, double base_hit_radius) const;
    int enemy_index_by_uid(const Array& enemies, int enemy_uid) const;
    bool projectile_segment_hits_point(const Vector2& previous_pos, const Vector2& pos, const Vector2& target_pos, double hit_radius) const;
};

}

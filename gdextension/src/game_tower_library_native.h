#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class GameTowerLibraryNative : public RefCounted {
    GDCLASS(GameTowerLibraryNative, RefCounted)

protected:
    static void _bind_methods();

public:
    static constexpr int MAX_LEVEL = 3;
    static constexpr double DAMAGE_LEVEL_BONUS = 0.32;
    static constexpr double RATE_LEVEL_BONUS = 0.18;
    static constexpr double RANGE_LEVEL_BONUS = 0.08;
    static constexpr double SELL_REFUND_RATIO = 0.60;

    Array tower_order() const;
    int max_level() const { return MAX_LEVEL; }
    Dictionary config(const String& tower_type) const;
    Dictionary info(const String& tower_type) const;
    int level(const Dictionary& tower) const;
    Dictionary stats_for_level(const String& tower_type, int tower_level) const;
    Dictionary runtime_stats(const Dictionary& tower) const;
    Dictionary runtime_stats_with_modifiers(const Dictionary& tower, const Array& unlocked_tech, const Dictionary& draft_package) const;
    double tech_slow_duration(const String& source, const Array& unlocked_tech) const;
    int slingshot_cost(int base_cost, const Array& unlocked_tech) const;
    int tower_cost(const String& tower_type) const;
    int upgrade_cost(const Dictionary& tower) const;
    int total_spent(const Dictionary& tower) const;
    int sell_refund(const Dictionary& tower) const;
    String short_label(const String& tower_type) const;
    String selected_readout(const String& tower_type, bool live_build) const;
    Dictionary managed_view_data(const Dictionary& tower, const Array& rings, int sol_credits) const;
    Dictionary button_view_data(const String& selected_tower, bool can_build, const Dictionary& tower_textures, int sol_credits) const;
};

}

#include "game_catalog_native.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>

using namespace godot;

namespace {

Array paths(std::initializer_list<const char*> values) {
    Array result;
    for (const char* value : values) {
        result.append(String(value));
    }
    return result;
}

Dictionary ring(int id, const String& name, double radius, double period, int slots, const String& best) {
    Dictionary data;
    data["id"] = id;
    data["name"] = name;
    data["radius"] = radius;
    data["period"] = period;
    data["slots"] = slots;
    data["best"] = best;
    return data;
}

Dictionary enemy(int variant_id, const String& label, double hp, double speed, double damage, int reward, double radius, double draw_size, const Color& color) {
    Dictionary data;
    data["variant_id"] = variant_id;
    data["label"] = label;
    data["hp"] = hp;
    data["speed"] = speed;
    data["damage"] = damage;
    data["reward"] = reward;
    data["radius"] = radius;
    data["draw_size"] = draw_size;
    data["color"] = color;
    return data;
}

Dictionary draft_package(const String& id, const String& title, const String& body, const Array& towers = Array(), const String& stat = "", double multiplier = 1.0, int sol = 0) {
    Dictionary data;
    data["id"] = id;
    data["title"] = title;
    data["body"] = body;
    if (!towers.is_empty()) {
        data["towers"] = towers;
    }
    if (!stat.is_empty()) {
        data[stat] = multiplier;
    }
    if (sol > 0) {
        data["sol"] = sol;
    }
    return data;
}

}

void GameCatalogNative::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_max_waves"), &GameCatalogNative::get_max_waves);
    ClassDB::bind_method(D_METHOD("get_sun_radius"), &GameCatalogNative::get_sun_radius);
    ClassDB::bind_method(D_METHOD("get_sun_damage_radius"), &GameCatalogNative::get_sun_damage_radius);
    ClassDB::bind_method(D_METHOD("get_enemy_spawn_padding"), &GameCatalogNative::get_enemy_spawn_padding);
    ClassDB::bind_method(D_METHOD("get_slot_angle_offset"), &GameCatalogNative::get_slot_angle_offset);
    ClassDB::bind_method(D_METHOD("get_flare_damage"), &GameCatalogNative::get_flare_damage);
    ClassDB::bind_method(D_METHOD("get_flare_overcharge_cost"), &GameCatalogNative::get_flare_overcharge_cost);
    ClassDB::bind_method(D_METHOD("get_flare_overcharge_damage_bonus"), &GameCatalogNative::get_flare_overcharge_damage_bonus);
    ClassDB::bind_method(D_METHOD("get_burrower_dig_radius"), &GameCatalogNative::get_burrower_dig_radius);
    ClassDB::bind_method(D_METHOD("get_burrower_excavation_hp"), &GameCatalogNative::get_burrower_excavation_hp);
    ClassDB::bind_method(D_METHOD("get_burrower_drain_interval"), &GameCatalogNative::get_burrower_drain_interval);
    ClassDB::bind_method(D_METHOD("get_burrower_drain_damage"), &GameCatalogNative::get_burrower_drain_damage);
    ClassDB::bind_method(D_METHOD("get_enemy_gravity_const"), &GameCatalogNative::get_enemy_gravity_const);
    ClassDB::bind_method(D_METHOD("get_enemy_gravity_accel_cap"), &GameCatalogNative::get_enemy_gravity_accel_cap);
    ClassDB::bind_method(D_METHOD("get_physics_projectile_gravity_const"), &GameCatalogNative::get_physics_projectile_gravity_const);
    ClassDB::bind_method(D_METHOD("get_physics_projectile_damage_ring_mult"), &GameCatalogNative::get_physics_projectile_damage_ring_mult);
    ClassDB::bind_method(D_METHOD("get_physics_projectile_outward_deflect"), &GameCatalogNative::get_physics_projectile_outward_deflect);
    ClassDB::bind_method(D_METHOD("get_physics_projectile_max_lifetime"), &GameCatalogNative::get_physics_projectile_max_lifetime);
    ClassDB::bind_method(D_METHOD("get_physics_projectile_hit_radius"), &GameCatalogNative::get_physics_projectile_hit_radius);
    ClassDB::bind_method(D_METHOD("get_slingshot_cost"), &GameCatalogNative::get_slingshot_cost);
    ClassDB::bind_method(D_METHOD("enemy_asset_paths"), &GameCatalogNative::enemy_asset_paths);
    ClassDB::bind_method(D_METHOD("enemy_animation_paths"), &GameCatalogNative::enemy_animation_paths);
    ClassDB::bind_method(D_METHOD("enemy_animation_base_angles"), &GameCatalogNative::enemy_animation_base_angles);
    ClassDB::bind_method(D_METHOD("enemy_masses"), &GameCatalogNative::enemy_masses);
    ClassDB::bind_method(D_METHOD("tower_asset_paths"), &GameCatalogNative::tower_asset_paths);
    ClassDB::bind_method(D_METHOD("rings"), &GameCatalogNative::rings);
    ClassDB::bind_method(D_METHOD("draft_packages"), &GameCatalogNative::draft_packages);
    ClassDB::bind_method(D_METHOD("draft_package_title", "draft_package"), &GameCatalogNative::draft_package_title);
    ClassDB::bind_method(D_METHOD("enemy_configs"), &GameCatalogNative::enemy_configs);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_waves"), "", "get_max_waves");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sun_radius"), "", "get_sun_radius");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sun_damage_radius"), "", "get_sun_damage_radius");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "enemy_spawn_padding"), "", "get_enemy_spawn_padding");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "slot_angle_offset"), "", "get_slot_angle_offset");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "flare_damage"), "", "get_flare_damage");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "flare_overcharge_cost"), "", "get_flare_overcharge_cost");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "flare_overcharge_damage_bonus"), "", "get_flare_overcharge_damage_bonus");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "burrower_dig_radius"), "", "get_burrower_dig_radius");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "burrower_excavation_hp"), "", "get_burrower_excavation_hp");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "burrower_drain_interval"), "", "get_burrower_drain_interval");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "burrower_drain_damage"), "", "get_burrower_drain_damage");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "enemy_gravity_const"), "", "get_enemy_gravity_const");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "enemy_gravity_accel_cap"), "", "get_enemy_gravity_accel_cap");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "physics_projectile_gravity_const"), "", "get_physics_projectile_gravity_const");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "physics_projectile_damage_ring_mult"), "", "get_physics_projectile_damage_ring_mult");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "physics_projectile_outward_deflect"), "", "get_physics_projectile_outward_deflect");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "physics_projectile_max_lifetime"), "", "get_physics_projectile_max_lifetime");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "physics_projectile_hit_radius"), "", "get_physics_projectile_hit_radius");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "slingshot_cost"), "", "get_slingshot_cost");
}

Dictionary GameCatalogNative::enemy_asset_paths() const {
    Dictionary data;
    data["drifter"] = "res://assets/sprites/clean/enemies_optimized/drifter_idle_1.png";
    data["bloom"] = "res://assets/sprites/clean/enemies_optimized/bloom_idle_1.png";
    data["burrower"] = "res://assets/sprites/clean/enemies_optimized/coronal_idle_1.png";
    data["mimic"] = "res://assets/sprites/clean/enemies/photon_idle_1.png";
    data["farmer"] = "res://assets/sprites/clean/enemies_optimized/solar_idle_1.png";
    data["prime"] = "res://assets/sprites/clean/enemies/astrophage-shell_idle_1.png";
    return data;
}

Dictionary GameCatalogNative::enemy_animation_paths() const {
    Dictionary data;
    Dictionary drifter;
    drifter["idle"] = paths({"res://assets/sprites/clean/enemies_optimized/drifter_idle_1.png", "res://assets/sprites/clean/enemies_optimized/drifter_idle_2.png"});
    drifter["move"] = paths({"res://assets/sprites/clean/enemies_optimized/drifter_move_1.png", "res://assets/sprites/clean/enemies_optimized/drifter_move_2.png", "res://assets/sprites/clean/enemies_optimized/drifter_move_3.png"});
    data["drifter"] = drifter;

    Dictionary bloom;
    bloom["idle"] = paths({"res://assets/sprites/clean/enemies_optimized/bloom_idle_1.png", "res://assets/sprites/clean/enemies_optimized/bloom_idle_2.png"});
    bloom["move"] = paths({"res://assets/sprites/clean/enemies_optimized/bloom_move_1.png", "res://assets/sprites/clean/enemies_optimized/bloom_move_2.png", "res://assets/sprites/clean/enemies_optimized/bloom_move_3.png"});
    data["bloom"] = bloom;

    Dictionary burrower;
    burrower["idle"] = paths({"res://assets/sprites/clean/enemies_optimized/coronal_idle_1.png", "res://assets/sprites/clean/enemies_optimized/coronal_idle_2.png"});
    burrower["move"] = paths({"res://assets/sprites/clean/enemies_optimized/coronal_move_1.png", "res://assets/sprites/clean/enemies_optimized/coronal_move_2.png", "res://assets/sprites/clean/enemies_optimized/coronal_move_3.png", "res://assets/sprites/clean/enemies_optimized/coronal_move_4.png"});
    data["burrower"] = burrower;

    Dictionary farmer;
    farmer["idle"] = paths({"res://assets/sprites/clean/enemies_optimized/solar_idle_1.png", "res://assets/sprites/clean/enemies_optimized/solar_idle_2.png"});
    farmer["move"] = paths({"res://assets/sprites/clean/enemies_optimized/solar_move_1.png", "res://assets/sprites/clean/enemies_optimized/solar_move_2.png", "res://assets/sprites/clean/enemies_optimized/solar_move_3.png"});
    data["farmer"] = farmer;

    Dictionary mimic;
    mimic["idle"] = paths({"res://assets/sprites/clean/enemies/photon_idle_1.png", "res://assets/sprites/clean/enemies/photon_idle_2.png"});
    mimic["move"] = paths({"res://assets/sprites/clean/enemies/photon_idle_1.png", "res://assets/sprites/clean/enemies/photon_idle_2.png"});
    data["mimic"] = mimic;

    Dictionary prime;
    Array shell_move = paths({"res://assets/sprites/clean/enemies_optimized/astrophage-shell_move_1.png", "res://assets/sprites/clean/enemies_optimized/astrophage-shell_move_2.png", "res://assets/sprites/clean/enemies_optimized/astrophage-shell_move_3.png", "res://assets/sprites/clean/enemies_optimized/astrophage-shell_move_4.png"});
    prime["idle"] = paths({"res://assets/sprites/clean/enemies_optimized/astrophage-shell_idle_1.png", "res://assets/sprites/clean/enemies_optimized/astrophage-shell_idle_2.png"});
    prime["move"] = shell_move;
    prime["active"] = paths({"res://assets/sprites/clean/enemies_optimized/astrophage-active_move_1.png", "res://assets/sprites/clean/enemies_optimized/astrophage-active_move_2.png", "res://assets/sprites/clean/enemies_optimized/astrophage-active_move_3.png", "res://assets/sprites/clean/enemies_optimized/astrophage-active_move_4.png"});
    prime["frenzy"] = paths({"res://assets/sprites/clean/enemies_optimized/astrophage-frenzy_move_1.png", "res://assets/sprites/clean/enemies_optimized/astrophage-frenzy_move_2.png", "res://assets/sprites/clean/enemies_optimized/astrophage-frenzy_move_3.png"});
    data["prime"] = prime;
    return data;
}

Dictionary GameCatalogNative::enemy_animation_base_angles() const {
    Dictionary data;
    data["drifter"] = 0.0;
    data["bloom"] = 0.0;
    data["burrower"] = -1.57079632679489661923;
    data["mimic"] = 0.0;
    data["farmer"] = -0.78539816339744830962;
    data["prime"] = 0.0;
    return data;
}

Dictionary GameCatalogNative::enemy_masses() const {
    Dictionary data;
    data["drifter"] = 1.0;
    data["bloom"] = 1.5;
    data["burrower"] = 3.0;
    data["mimic"] = 0.8;
    data["farmer"] = 1.2;
    data["prime"] = 8.0;
    return data;
}

Dictionary GameCatalogNative::tower_asset_paths() const {
    Dictionary data;
    data["photon_splitter"] = "res://assets/sprites/clean/towers/photon_splitter.png";
    data["cryo_probe"] = "res://assets/sprites/clean/towers/cryo_probe.png";
    data["bio_lab"] = "res://assets/sprites/clean/towers/bio_lab.png";
    data["magnetic_net"] = "res://assets/sprites/clean/towers/magnetic_net.png";
    data["helios_cannon"] = "res://assets/sprites/clean/towers/helios_cannon.png";
    data["tardigrade_bomb"] = "res://assets/sprites/clean/towers/tardigrade_bomb.png";
    return data;
}

Array GameCatalogNative::rings() const {
    Array data;
    data.append(ring(1, "Corona Belt", 80.0, 6.0, 4, "Photon Splitter, Helios Cannon"));
    data.append(ring(2, "Chromosphere Band", 140.0, 11.0, 6, "Cryo Probe, Tardigrade Bomb"));
    data.append(ring(3, "Photosphere Arc", 210.0, 17.0, 8, "Bio-Lab Station, Magnetic Net"));
    data.append(ring(4, "Outer Veil", 290.0, 26.0, 10, "Early intercept"));
    return data;
}

Array GameCatalogNative::draft_packages() const {
    Array data;
    data.append(draft_package("sol", "Sol Windfall", "+55 Sol Credits before Wave 1.", Array(), "", 1.0, 55));
    data.append(draft_package("control", "Control Grid", "Cryo and Magnetic gain +10% range.", Array::make("cryo_probe", "magnetic_net"), "range", 1.10));
    data.append(draft_package("burst", "Burst Lenses", "Photon and Helios gain +10% damage.", Array::make("photon_splitter", "helios_cannon"), "damage", 1.10));
    data.append(draft_package("bio", "Bio Bloom", "Bio-Lab and Tardigrade gain +10% rate.", Array::make("bio_lab", "tardigrade_bomb"), "rate", 1.10));
    return data;
}

String GameCatalogNative::draft_package_title(const Dictionary& draft_package) const {
    return String(draft_package.get("title", "Pick a contract"));
}

Dictionary GameCatalogNative::enemy_configs() const {
    Dictionary data;
    data["drifter"] = enemy(0, "Drifter", 30.0, 45.0, 0.045, 7, 15.0, 46.0, Color(0.96, 0.42, 0.48));
    data["bloom"] = enemy(1, "Bloom", 62.0, 40.0, 0.045, 13, 18.0, 54.0, Color(1.0, 0.62, 0.36));
    data["burrower"] = enemy(2, "Coronal Burrower", 105.0, 29.0, 0.065, 26, 19.0, 58.0, Color(0.76, 0.50, 0.30));
    data["mimic"] = enemy(3, "Photon Mimic", 54.0, 46.0, 0.045, 18, 16.0, 48.0, Color(0.70, 0.62, 0.98));
    data["farmer"] = enemy(4, "Solar Farmer", 48.0, 42.0, 0.045, 17, 17.0, 50.0, Color(0.55, 0.92, 0.45));
    data["prime"] = enemy(5, "Astrophage Prime", 480.0, 22.0, 0.10, 150, 34.0, 84.0, Color(1.0, 0.18, 0.15));
    return data;
}

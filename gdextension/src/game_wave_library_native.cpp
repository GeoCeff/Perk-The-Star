#include "game_wave_library_native.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <algorithm>
#include <cmath>

using namespace godot;

namespace {

Array variant_keys() {
    Array keys;
    keys.append("drifter");
    keys.append("bloom");
    keys.append("burrower");
    keys.append("mimic");
    keys.append("farmer");
    keys.append("prime");
    keys.append("splitter_bloom");
    return keys;
}

bool is_number(const Variant& value) {
    return value.get_type() == Variant::INT || value.get_type() == Variant::FLOAT;
}

int int_value(const Variant& value, int fallback = 0) {
    if (is_number(value)) {
        return static_cast<int>(static_cast<int64_t>(value));
    }
    return fallback;
}

double float_value(const Variant& value, double fallback = 0.0) {
    if (is_number(value)) {
        return static_cast<double>(value);
    }
    return fallback;
}

String enemy_label(const String& variant) {
    if (variant == "bloom") return "Bloom";
    if (variant == "splitter_bloom") return "Splitter Bloom";
    if (variant == "burrower") return "Coronal Burrower";
    if (variant == "mimic") return "Photon Mimic";
    if (variant == "farmer") return "Solar Farmer";
    if (variant == "prime") return "Astrophage Prime";
    return "Drifter";
}

}

void GameWaveLibraryNative::_bind_methods() {
    ClassDB::bind_method(D_METHOD("load_wave", "wave_number"), &GameWaveLibraryNative::load_wave);
    ClassDB::bind_method(D_METHOD("normalize_wave_data", "data", "wave_number"), &GameWaveLibraryNative::normalize_wave_data);
    ClassDB::bind_method(D_METHOD("build_spawn_queue", "wave_data"), &GameWaveLibraryNative::build_spawn_queue);
    ClassDB::bind_method(D_METHOD("variant_key", "raw"), &GameWaveLibraryNative::variant_key);
    ClassDB::bind_method(D_METHOD("primary_variant", "wave_data"), &GameWaveLibraryNative::primary_variant);
    ClassDB::bind_method(D_METHOD("spawn_summary", "wave_data"), &GameWaveLibraryNative::spawn_summary);
    ClassDB::bind_method(D_METHOD("warning_tags", "wave_data"), &GameWaveLibraryNative::warning_tags);
    ClassDB::bind_method(D_METHOD("counter_hint", "wave_data"), &GameWaveLibraryNative::counter_hint);
    ClassDB::bind_method(D_METHOD("intel_detail", "wave_data", "reward", "active_count", "burrowed_count", "queued_count", "modifier_summary"), &GameWaveLibraryNative::intel_detail, DEFVAL(-1), DEFVAL(0), DEFVAL(0), DEFVAL(""));
    ClassDB::bind_method(D_METHOD("clean_hint", "text", "wave_name"), &GameWaveLibraryNative::clean_hint);
    ClassDB::bind_method(D_METHOD("enemy_short_label", "variant"), &GameWaveLibraryNative::enemy_short_label);
    ClassDB::bind_method(D_METHOD("total_spawn_count", "wave_data"), &GameWaveLibraryNative::total_spawn_count);
    ClassDB::bind_method(D_METHOD("preview_label", "wave_data"), &GameWaveLibraryNative::preview_label);
    ClassDB::bind_method(D_METHOD("banner_data", "wave_data"), &GameWaveLibraryNative::banner_data);
    ClassDB::bind_method(D_METHOD("array_value", "value"), &GameWaveLibraryNative::array_value);
    ClassDB::bind_method(D_METHOD("draft_defense_wave_data", "wave_number", "draft_package_title", "max_waves"), &GameWaveLibraryNative::draft_defense_wave_data, DEFVAL(12));
    ClassDB::bind_method(D_METHOD("daily_seed_label"), &GameWaveLibraryNative::daily_seed_label);
    ClassDB::bind_method(D_METHOD("daily_seed_value"), &GameWaveLibraryNative::daily_seed_value);
    ClassDB::bind_method(D_METHOD("daily_seed_wave_data", "wave_number"), &GameWaveLibraryNative::daily_seed_wave_data);
    ClassDB::bind_method(D_METHOD("boss_rush_wave_data", "wave_number"), &GameWaveLibraryNative::boss_rush_wave_data);
    ClassDB::bind_method(D_METHOD("endless_wave_data", "wave_number"), &GameWaveLibraryNative::endless_wave_data);
    ClassDB::bind_method(D_METHOD("repeat_variant", "variant", "count"), &GameWaveLibraryNative::repeat_variant);
}

Dictionary GameWaveLibraryNative::load_wave(int wave_number) const {
    const String path = vformat("res://data/waves/wave_%02d.json", wave_number);
    Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);
    if (file.is_null()) {
        return Dictionary();
    }

    Variant parsed = JSON::parse_string(file->get_as_text());
    if (parsed.get_type() != Variant::DICTIONARY) {
        return Dictionary();
    }
    return normalize_wave_data(parsed, wave_number);
}

Dictionary GameWaveLibraryNative::normalize_wave_data(const Dictionary& data, int wave_number) const {
    const double default_interval = std::max(float_value(data.get("spawn_interval", 2.0), 2.0), 0.05);
    Array spawns;
    Array clash_groups;
    String wave_type = String(data.get("wave_type", "normal")).strip_edges().to_lower();
    if (wave_type != "normal" && wave_type != "formation" && wave_type != "clash" && wave_type != "boss") {
        wave_type = "normal";
    }

    Variant event_value = data.get("event", Dictionary());
    Dictionary event_data = event_value.get_type() == Variant::DICTIONARY ? Dictionary(event_value) : Dictionary();
    Variant formation_value = data.get("formation", Dictionary());
    Dictionary formation_data = formation_value.get_type() == Variant::DICTIONARY ? Dictionary(formation_value) : Dictionary();

    if (data.has("spawns")) {
        const Array raw_spawns = array_value(data.get("spawns", Array()));
        for (int i = 0; i < raw_spawns.size(); ++i) {
            if (raw_spawns[i].get_type() != Variant::DICTIONARY) {
                continue;
            }
            Dictionary entry = raw_spawns[i];
            Dictionary spawn;
            spawn["variant"] = variant_key(entry.get("variant", 0));
            spawn["count"] = std::max(0, int_value(entry.get("count", 0)));
            spawn["interval"] = std::max(float_value(entry.get("interval", default_interval), default_interval), 0.05);
            spawns.append(spawn);
        }
    } else if (data.has("enemies")) {
        const Array enemies = array_value(data.get("enemies", Array()));
        for (int i = 0; i < enemies.size(); ++i) {
            if (enemies[i].get_type() != Variant::DICTIONARY) {
                continue;
            }
            Dictionary entry = enemies[i];
            Dictionary spawn;
            spawn["variant"] = variant_key(entry.get("variant", "drifter"));
            spawn["count"] = std::max(0, int_value(entry.get("count", 0)));
            spawn["interval"] = default_interval;
            spawns.append(spawn);
        }
    }

    const Array raw_groups = array_value(data.get("clash_groups", Array()));
    for (int i = 0; i < raw_groups.size(); ++i) {
        if (raw_groups[i].get_type() != Variant::DICTIONARY) {
            continue;
        }
        Dictionary raw_group = raw_groups[i];
        Array variants;
        const Array raw_variants = array_value(raw_group.get("variants", Array()));
        for (int j = 0; j < raw_variants.size(); ++j) {
            variants.append(variant_key(raw_variants[j]));
        }
        if (variants.is_empty()) {
            continue;
        }
        Dictionary group;
        group["variants"] = variants;
        group["spawn_pattern"] = String(raw_group.get("spawn_pattern", "random")).strip_edges().to_lower();
        group["delay_before"] = std::max(float_value(raw_group.get("delay_before", 0.0), 0.0), 0.0);
        if (raw_group.has("spread_angle_deg")) {
            group["spread_angle_deg"] = float_value(raw_group.get("spread_angle_deg", 60.0), 60.0);
        }
        if (raw_group.has("spiral_arms")) {
            group["spiral_arms"] = std::max(1, int_value(raw_group.get("spiral_arms", 1)));
        }
        clash_groups.append(group);
    }

    if (!formation_data.is_empty()) {
        Array formation_variants;
        const Array raw_variants = array_value(formation_data.get("variants", Array::make("drifter")));
        for (int i = 0; i < raw_variants.size(); ++i) {
            formation_variants.append(variant_key(raw_variants[i]));
        }
        if (formation_variants.is_empty()) {
            formation_variants.append("drifter");
        }
        Dictionary normalized;
        normalized["type"] = String(formation_data.get("type", "ring")).strip_edges().to_lower();
        normalized["variants"] = formation_variants;
        normalized["count"] = std::max(0, int_value(formation_data.get("count", 0)));
        normalized["spread_angle_deg"] = float_value(formation_data.get("spread_angle_deg", 60.0), 60.0);
        normalized["spiral_arms"] = std::max(1, int_value(formation_data.get("spiral_arms", 1)));
        formation_data = normalized;
    }

    Dictionary normalized;
    normalized["index"] = int_value(data.get("index", data.get("wave", wave_number)), wave_number);
    normalized["name"] = String(data.get("name", vformat("Wave %02d", wave_number)));
    normalized["wave_type"] = wave_type;
    normalized["spawn_interval"] = default_interval;
    normalized["credit_reward"] = int_value(data.get("credit_reward", data.get("reward_base", 0)));
    normalized["spawns"] = spawns;
    normalized["clash_groups"] = clash_groups;
    normalized["formation"] = formation_data;
    normalized["event"] = event_data;
    normalized["escalation_threshold_seconds"] = data.get("escalation_threshold_seconds", Variant());
    normalized["tutorial_hint"] = String(data.get("tutorial_hint", "Defend the Sun."));
    return normalized;
}

Array GameWaveLibraryNative::build_spawn_queue(const Dictionary& wave_data) const {
    Array queue;
    const String wave_type = String(wave_data.get("wave_type", "normal"));
    if (wave_type == "clash" || wave_type == "boss") {
        return queue;
    }
    const Array entries = spawn_entries(wave_data);
    for (int i = 0; i < entries.size(); ++i) {
        if (entries[i].get_type() != Variant::DICTIONARY) {
            continue;
        }
        Dictionary entry = entries[i];
        const String variant = variant_key(entry.get("variant", 0));
        const int count = std::max(0, int_value(entry.get("count", 0)));
        const double interval = std::max(float_value(entry.get("interval", 2.0), 2.0), 0.05);
        for (int j = 0; j < count; ++j) {
            Dictionary spawn;
            spawn["variant"] = variant;
            spawn["interval"] = interval;
            queue.append(spawn);
        }
    }
    return queue;
}

String GameWaveLibraryNative::variant_key(const Variant& raw) const {
    if (is_number(raw)) {
        const int idx = int_value(raw);
        const Array keys = variant_keys();
        if (idx >= 0 && idx < keys.size()) {
            return keys[idx];
        }
        return "drifter";
    }

    String cleaned = String(raw).strip_edges().to_lower();
    if (cleaned.is_valid_int()) {
        return variant_key(cleaned.to_int());
    }
    cleaned = cleaned.replace(" ", "_").replace("-", "_");
    if (cleaned == "drifter") return "drifter";
    if (cleaned == "bloom") return "bloom";
    if (cleaned == "splitter_bloom" || cleaned == "splitter bloom") return "splitter_bloom";
    if (cleaned == "burrower" || cleaned == "coronal_burrower") return "burrower";
    if (cleaned == "mimic" || cleaned == "photon_mimic") return "mimic";
    if (cleaned == "farmer" || cleaned == "solar_farmer") return "farmer";
    if (cleaned == "prime" || cleaned == "astrophage_prime") return "prime";
    return "drifter";
}

String GameWaveLibraryNative::primary_variant(const Dictionary& wave_data) const {
    Dictionary counts = variant_counts(wave_data);
    if (counts.is_empty()) {
        return "drifter";
    }
    return String(counts.keys()[0]);
}

String GameWaveLibraryNative::spawn_summary(const Dictionary& wave_data) const {
    Array parts;
    Dictionary counts = variant_counts(wave_data);
    const Array keys = counts.keys();
    for (int i = 0; i < keys.size(); ++i) {
        const String variant = String(keys[i]);
        parts.append(vformat("%d %s", int_value(counts[variant]), enemy_short_label(variant)));
    }
    return parts.is_empty() ? String("No spawns loaded") : String(", ").join(parts);
}

String GameWaveLibraryNative::warning_tags(const Dictionary& wave_data) const {
    Array tags;
    Dictionary seen;
    const String wave_type = String(wave_data.get("wave_type", "normal"));
    if (wave_type == "clash") tags.append("CLASH");
    else if (wave_type == "formation") tags.append("FORMATION");
    else if (wave_type == "boss") tags.append("BOSS");

    Dictionary counts = variant_counts(wave_data);
    const Array keys = counts.keys();
    for (int i = 0; i < keys.size(); ++i) {
        const String variant = variant_key(keys[i]);
        if (seen.has(variant)) {
            continue;
        }
        seen[variant] = true;
        if (variant == "bloom") tags.append("SPLIT");
        else if (variant == "splitter_bloom") tags.append("SPLITTER");
        else if (variant == "burrower") tags.append("BURROW");
        else if (variant == "mimic") tags.append("MIMIC");
        else if (variant == "farmer") tags.append("ABSORB");
        else if (variant == "prime") tags.append("PRIME");
    }

    Variant event_value = wave_data.get("event", Dictionary());
    String event_type;
    if (event_value.get_type() == Variant::DICTIONARY) {
        Dictionary event_data = event_value;
        event_type = String(event_data.get("type", ""));
    }
    if (event_type == "mid_wave_autoflare") tags.append("STORM");
    else if (event_type == "ring_blind") tags.append("RING DARK");
    else if (event_type == "bio_lab_boost") tags.append("BIO BOOST");
    else if (event_type == "prime_frenzy") tags.append("FRENZY");
    else if (event_type == "comet_cache") tags.append("CACHE");

    return tags.is_empty() ? String("TAGS: BASIC SWARM") : vformat("TAGS: %s", String("  |  ").join(tags));
}

String GameWaveLibraryNative::counter_hint(const Dictionary& wave_data) const {
    Dictionary variants = variant_counts(wave_data);
    if (variants.has("prime")) return "ADVISOR: Bio-Lab opens Prime shell; Helios or Tardigrade finishes the core.";
    if (variants.has("farmer")) return "ADVISOR: Slow Farmers with Cryo/Magnetic before using energy damage.";
    if (variants.has("mimic")) return "ADVISOR: Mimics ignore Photon; mix Bio-Lab, Cryo, Magnetic, or Helios.";
    if (variants.has("burrower")) return "ADVISOR: Build Bio-Lab coverage before Burrowers reach the corona.";
    if (variants.has("splitter_bloom")) return "ADVISOR: Burst Splitter Blooms early or slow the Drifters they release.";
    if (variants.has("bloom")) return "ADVISOR: Slow Blooms first; they split into Drifters when destroyed.";
    Variant event_value = wave_data.get("event", Dictionary());
    if (event_value.get_type() == Variant::DICTIONARY) {
        Dictionary event_data = event_value;
        if (String(event_data.get("type", "")) == "comet_cache") return "ADVISOR: Outer-ring fire can crack the Comet Cache for bonus Sol.";
    }
    return "ADVISOR: Photon Splitters handle Drifters; place one where its orbit crosses the lane.";
}

String GameWaveLibraryNative::intel_detail(const Dictionary& wave_data, int reward, int active_count, int burrowed_count, int queued_count, const String& modifier_summary) const {
    Array lines;
    lines.append(vformat("%s | CONTACTS %d | REWARD +%d SOL", type_label(wave_data), total_spawn_count(wave_data), reward));
    lines.append(warning_tags(wave_data));
    lines.append(counter_hint(wave_data));
    Variant event_value = wave_data.get("event", Dictionary());
    if (event_value.get_type() == Variant::DICTIONARY) {
        Dictionary event_data = event_value;
        if (String(event_data.get("type", "")) == "comet_cache") lines.append("EVENT: Destroy Comet Cache for Sol, or it breaks into Drifters.");
        else if (String(event_data.get("type", "")) == "mid_wave_autoflare") lines.append("EVENT: Solar storm fires a flare and briefly disrupts Cryo.");
        else if (String(event_data.get("type", "")) == "ring_blind") lines.append("EVENT: Dark rings stop towers there; cover with other orbits.");
        else if (String(event_data.get("type", "")) == "bio_lab_boost") lines.append("EVENT: Bio-Labs surge; use them to clean hard targets.");
    }
    if (active_count >= 0) {
        lines.insert(0, vformat("LIVE: %d ACTIVE | %d BURROWED | %d QUEUED", active_count, burrowed_count, queued_count));
    }
    if (modifier_summary.strip_edges() != "") {
        lines.append(modifier_summary.strip_edges());
    }
    return String("\n").join(lines);
}

String GameWaveLibraryNative::clean_hint(const String& text, const String& wave_name) const {
    const String repeated_prefix = vformat("%s: ", wave_name);
    if (text.begins_with(repeated_prefix)) {
        return text.substr(repeated_prefix.length());
    }
    return text;
}

String GameWaveLibraryNative::enemy_short_label(const String& variant) const {
    if (variant == "burrower") return "Burrower";
    if (variant == "splitter_bloom") return "Splitter";
    if (variant == "mimic") return "Mimic";
    if (variant == "farmer") return "Farmer";
    if (variant == "prime") return "Prime";
    return enemy_label(variant);
}

int GameWaveLibraryNative::total_spawn_count(const Dictionary& wave_data) const {
    int count = 0;
    const Array values = variant_counts(wave_data).values();
    for (int i = 0; i < values.size(); ++i) {
        count += std::max(0, int_value(values[i]));
    }
    return count;
}

String GameWaveLibraryNative::preview_label(const Dictionary& wave_data) const {
    const int count = total_spawn_count(wave_data);
    const String wave_type = String(wave_data.get("wave_type", "normal"));
    if (wave_type == "clash") return vformat("Massive wave approaching - %d enemies", count);
    if (wave_type == "boss") return vformat("Astrophage Prime detected - %d contacts", count);
    if (wave_type == "formation") return vformat("Formation wave incoming - %d enemies", count);
    return vformat("Wave incoming - %d enemies", count);
}

Dictionary GameWaveLibraryNative::banner_data(const Dictionary& wave_data) const {
    Color accent(1.0, 0.86, 0.34);
    const String wave_type = String(wave_data.get("wave_type", "normal"));
    if (wave_type == "clash") {
        accent = Color(1.0, 0.32, 0.12);
    } else if (wave_type == "boss") {
        accent = Color(1.0, 0.12, 0.12);
    } else if (wave_type == "formation") {
        accent = Color(0.42, 0.90, 1.0);
    }

    Dictionary data;
    data["title"] = preview_label(wave_data);
    data["subtitle"] = String(wave_data.get("name", "Next Wave"));
    data["accent"] = accent;
    data["duration"] = 4.0;
    return data;
}

Array GameWaveLibraryNative::array_value(const Variant& value) const {
    if (value.get_type() == Variant::ARRAY) {
        return value;
    }
    return Array();
}

Dictionary GameWaveLibraryNative::draft_defense_wave_data(int wave_number, const String& draft_package_title, int max_waves) const {
    const int wave = std::max(1, std::min(6, wave_number));
    if (wave == 6) {
        Dictionary finale = boss_rush_wave_data(2).duplicate(true);
        finale["index"] = wave;
        finale["name"] = "Draft Prime";
        finale["credit_reward"] = 175;
        finale["enemy_hp_scale"] = 1.08;
        finale["enemy_speed_scale"] = 1.06;
        finale["enemy_damage_scale"] = 1.02;
        finale["enemy_reward_scale"] = 1.18;
        finale["tutorial_hint"] = vformat("Draft finale: %s must carry you through a Prime Echo and its escorts.", draft_package_title);
        return finale;
    }

    Dictionary data = load_wave(std::min(wave + 1, std::max(1, max_waves - 1))).duplicate(true);
    data["index"] = wave;
    data["name"] = vformat("Draft %s", String(data.get("name", vformat("Contact %d", wave))));
    data["credit_reward"] = int_value(data.get("credit_reward", 0)) + 18 + wave * 4;
    data["enemy_hp_scale"] = float_value(data.get("enemy_hp_scale", 1.0), 1.0) * (0.88 + double(wave) * 0.045);
    data["enemy_speed_scale"] = float_value(data.get("enemy_speed_scale", 1.0), 1.0) * (0.96 + double(wave) * 0.015);
    data["enemy_reward_scale"] = float_value(data.get("enemy_reward_scale", 1.0), 1.0) * 1.08;
    data["tutorial_hint"] = vformat("Draft Defense: %s active. %s", draft_package_title, String(data.get("tutorial_hint", "Hold the six-wave line.")));
    return data;
}

String GameWaveLibraryNative::daily_seed_label() const {
    const Dictionary date = Time::get_singleton()->get_date_dict_from_system();
    return vformat("%04d%02d%02d", int_value(date.get("year", 0)), int_value(date.get("month", 1)), int_value(date.get("day", 1)));
}

int GameWaveLibraryNative::daily_seed_value() const {
    return daily_seed_label().to_int();
}

Dictionary GameWaveLibraryNative::daily_seed_wave_data(int wave_number) const {
    const int wave = std::max(1, std::min(6, wave_number));
    const Array pool = Array::make("drifter", "bloom", "burrower", "mimic", "farmer");
    const String primary = String(daily_pick(pool, wave, 1));
    const String secondary = String(daily_pick(pool, wave, 2));
    const double hp_scale = 0.92 + double(wave) * 0.07;
    const double speed_scale = 0.96 + double(wave) * 0.025;
    const int reward = 52 + wave * 22;
    Dictionary data;
    data["index"] = wave;
    data["enemy_hp_scale"] = hp_scale;
    data["enemy_speed_scale"] = speed_scale;
    data["escalation_threshold_seconds"] = Variant();
    if (wave == 6) {
        data["name"] = vformat("Daily Prime %s", daily_seed_label());
        data["wave_type"] = "boss";
        data["spawn_interval"] = 0.72;
        data["credit_reward"] = reward + 80;
        Array groups;
        Dictionary group;
        group["variants"] = repeat_variant(primary, 10) + repeat_variant(secondary, 8);
        group["spawn_pattern"] = "ring";
        group["delay_before"] = 0.0;
        groups.append(group);
        group = Dictionary();
        group["variants"] = repeat_variant("burrower", 3) + repeat_variant("mimic", 4) + repeat_variant("farmer", 3);
        group["spawn_pattern"] = "v_shape";
        group["spread_angle_deg"] = 58;
        group["delay_before"] = 8.0;
        groups.append(group);
        group = Dictionary();
        group["variants"] = Array::make("prime");
        group["spawn_pattern"] = "center_top";
        group["delay_before"] = 20.0;
        groups.append(group);
        data["clash_groups"] = groups;
        Dictionary event;
        event["type"] = "bio_lab_boost";
        event["multiplier"] = 2.0;
        event["duration"] = 20.0;
        event["trigger_at_percent"] = 0.0;
        data["event"] = event;
        data["enemy_damage_scale"] = 1.0;
        data["enemy_reward_scale"] = 1.1;
        data["tutorial_hint"] = vformat("Daily finale %s: Prime arrives with today's fixed escort mix.", daily_seed_label());
        return data;
    }
    if (wave % 3 == 0) {
        data["name"] = vformat("Daily Vector %d", wave);
        data["wave_type"] = "formation";
        data["spawn_interval"] = 1.55;
        data["credit_reward"] = reward;
        Dictionary spawn;
        spawn["variant"] = primary;
        spawn["count"] = 8 + wave;
        spawn["interval"] = 1.15;
        data["spawns"] = Array::make(spawn);
        Dictionary formation;
        formation["type"] = "spiral";
        formation["variants"] = Array::make(primary, secondary);
        formation["count"] = 8 + wave;
        formation["spiral_arms"] = 2;
        data["formation"] = formation;
        data["event"] = Dictionary();
        data["enemy_damage_scale"] = 0.90 + double(wave) * 0.03;
        data["enemy_reward_scale"] = 1.0;
        data["tutorial_hint"] = vformat("Daily Seed %s: same formation all day, so adapt and replay for a cleaner clear.", daily_seed_label());
        return data;
    }
    data["name"] = vformat("Daily Contact %d", wave);
    data["wave_type"] = "normal";
    data["spawn_interval"] = std::max(0.9, 2.2 - double(wave) * 0.18);
    data["credit_reward"] = reward;
    Dictionary spawn_a;
    spawn_a["variant"] = primary;
    spawn_a["count"] = 7 + wave * 2;
    spawn_a["interval"] = std::max(0.9, 2.0 - double(wave) * 0.12);
    Dictionary spawn_b;
    spawn_b["variant"] = secondary;
    spawn_b["count"] = std::max(2, wave + 1);
    spawn_b["interval"] = 1.25;
    data["spawns"] = Array::make(spawn_a, spawn_b);
    Dictionary event;
    if (wave == 4) {
        event["type"] = "ring_blind";
        event["rings"] = Array::make(wave % 4);
        event["duration"] = 8.0;
        event["trigger_at_percent"] = 0.3;
    }
    data["event"] = event;
    data["enemy_damage_scale"] = 0.86 + double(wave) * 0.03;
    data["enemy_reward_scale"] = 1.0;
    data["tutorial_hint"] = vformat("Daily Seed %s: today's fixed enemy mix is %s plus %s.", daily_seed_label(), primary.capitalize(), secondary.capitalize());
    return data;
}

Dictionary GameWaveLibraryNative::boss_rush_wave_data(int wave_number) const {
    const int wave = std::max(1, std::min(4, wave_number));
    const int escort_count = 8 + wave * 4;
    const double prime_delay = 12.0 + double(wave) * 2.0;
    const double hp_scale = 0.62 + double(wave) * 0.16;
    const double speed_scale = 1.0 + double(wave - 1) * 0.05;
    Array groups;
    Dictionary group;
    group["variants"] = repeat_variant("drifter", escort_count) + repeat_variant("bloom", wave + 1);
    group["spawn_pattern"] = "ring";
    group["delay_before"] = 0.0;
    groups.append(group);
    group = Dictionary();
    group["variants"] = repeat_variant("burrower", std::max(0, wave - 1)) + repeat_variant("mimic", wave) + repeat_variant("farmer", std::max(0, wave - 2));
    group["spawn_pattern"] = "v_shape";
    group["spread_angle_deg"] = 48 + wave * 3;
    group["delay_before"] = 6.0;
    groups.append(group);
    group = Dictionary();
    group["variants"] = Array::make("prime");
    group["spawn_pattern"] = "center_top";
    group["delay_before"] = prime_delay;
    groups.append(group);

    Dictionary data;
    data["index"] = wave;
    data["name"] = vformat("Prime Echo %d", wave);
    data["wave_type"] = "boss";
    data["spawn_interval"] = std::max(0.55, 1.25 - double(wave) * 0.12);
    data["credit_reward"] = 90 + wave * 35;
    data["clash_groups"] = groups;
    Dictionary event;
    if (wave >= 3) {
        event["type"] = "bio_lab_boost";
        event["multiplier"] = 1.5 + double(wave) * 0.25;
        event["duration"] = 14.0 + double(wave) * 2.0;
        event["trigger_at_percent"] = 0.0;
    }
    data["event"] = event;
    data["enemy_hp_scale"] = hp_scale;
    data["enemy_speed_scale"] = speed_scale;
    data["enemy_damage_scale"] = 0.78 + double(wave) * 0.08;
    data["enemy_reward_scale"] = 1.2;
    data["escalation_threshold_seconds"] = Variant();
    data["tutorial_hint"] = vformat("Boss Rush: Prime Echo %d arrives fast. Crack the shell with Bio-Lab, then finish with Helios or Tardigrade.", wave);
    return data;
}

Dictionary GameWaveLibraryNative::endless_wave_data(int wave_number) const {
    const int wave = std::max(1, wave_number);
    const int drifters = 6 + wave * 2;
    const int blooms = std::max(0, wave - 2);
    const int burrowers = wave >= 5 ? 1 + int(std::floor(double(wave - 5) / 2.0)) : 0;
    const int mimics = wave >= 6 ? 1 + int(std::floor(double(wave - 6) / 2.0)) : 0;
    const int farmers = wave >= 8 ? 1 + int(std::floor(double(wave - 8) / 2.0)) : 0;
    const double interval = std::max(0.72, 2.8 - double(wave) * 0.075);
    const double hp_scale = 1.0 + std::max(0.0, double(wave - 6)) * 0.035;
    const double speed_scale = 1.0 + std::max(0.0, double(wave - 8)) * 0.012;
    const double damage_scale = 1.0 + std::max(0.0, double(wave - 10)) * 0.012;
    const double reward_scale = 1.0 + std::max(0.0, double(wave - 1)) * 0.012;
    const int credit_reward = 24 + wave * 10 + int(std::floor(double(wave) / 5.0)) * 18;

    Array spawns;
    Dictionary spawn;
    spawn["variant"] = "drifter";
    spawn["count"] = drifters;
    spawn["interval"] = interval;
    spawns.append(spawn);
    if (blooms > 0) {
        spawn = Dictionary();
        spawn["variant"] = "bloom";
        spawn["count"] = blooms;
        spawn["interval"] = interval + 0.15;
        spawns.append(spawn);
    }
    if (burrowers > 0) {
        spawn = Dictionary();
        spawn["variant"] = "burrower";
        spawn["count"] = burrowers;
        spawn["interval"] = interval + 0.30;
        spawns.append(spawn);
    }
    if (mimics > 0) {
        spawn = Dictionary();
        spawn["variant"] = "mimic";
        spawn["count"] = mimics;
        spawn["interval"] = interval + 0.20;
        spawns.append(spawn);
    }
    if (farmers > 0) {
        spawn = Dictionary();
        spawn["variant"] = "farmer";
        spawn["count"] = farmers;
        spawn["interval"] = interval + 0.25;
        spawns.append(spawn);
    }
    if (wave >= 11 && blooms >= 3) {
        spawn = Dictionary();
        spawn["variant"] = "splitter_bloom";
        spawn["count"] = std::max(1, int(std::floor(double(blooms) / 3.0)));
        spawn["interval"] = interval + 0.22;
        spawns.append(spawn);
    }

    String wave_type = "normal";
    String wave_name = "Endless Contact";
    Array clash_groups;
    Dictionary formation;
    if (wave % 10 == 0) {
        wave_type = "boss";
        wave_name = "Prime Echo";
        const int first_drifters = int(std::round(double(drifters) * 0.55));
        const int first_blooms = int(std::round(double(blooms) * 0.45));
        Dictionary group;
        group["variants"] = repeat_variant("drifter", first_drifters) + repeat_variant("bloom", first_blooms);
        group["spawn_pattern"] = "ring";
        group["delay_before"] = 0.0;
        clash_groups.append(group);
        group = Dictionary();
        group["variants"] = repeat_variant("burrower", burrowers) + repeat_variant("mimic", mimics) + repeat_variant("farmer", farmers);
        group["spawn_pattern"] = "v_shape";
        group["spread_angle_deg"] = 55;
        group["delay_before"] = 8.0;
        clash_groups.append(group);
        group = Dictionary();
        group["variants"] = repeat_variant("drifter", std::max(0, drifters - first_drifters)) + repeat_variant("bloom", std::max(0, blooms - first_blooms));
        group["spawn_pattern"] = "ring";
        group["delay_before"] = 17.0;
        clash_groups.append(group);
        group = Dictionary();
        group["variants"] = Array::make("prime");
        group["spawn_pattern"] = "center_top";
        group["delay_before"] = 26.0;
        clash_groups.append(group);
    } else if (wave % 5 == 0) {
        wave_type = "clash";
        wave_name = "Clash Surge";
        const int lead_drifters = int(std::round(double(drifters) * 0.60));
        Dictionary group;
        group["variants"] = repeat_variant("drifter", lead_drifters) + repeat_variant("bloom", int(std::round(double(blooms) * 0.50)));
        group["spawn_pattern"] = "ring";
        group["delay_before"] = 0.0;
        clash_groups.append(group);
        group = Dictionary();
        group["variants"] = repeat_variant("burrower", burrowers) + repeat_variant("mimic", mimics) + repeat_variant("farmer", farmers);
        group["spawn_pattern"] = "v_shape";
        group["spread_angle_deg"] = 50;
        group["delay_before"] = 8.0;
        clash_groups.append(group);
        group = Dictionary();
        group["variants"] = repeat_variant("drifter", std::max(0, drifters - lead_drifters)) + repeat_variant("bloom", int(std::floor(double(blooms) * 0.50)));
        group["spawn_pattern"] = "random";
        group["delay_before"] = 16.0;
        clash_groups.append(group);
    } else if (wave % 7 == 0) {
        wave_type = "formation";
        wave_name = "Vector Swarm";
        formation["type"] = wave >= 14 ? "spiral" : "v_shape";
        formation["variants"] = wave >= 7 ? Array::make("burrower", "mimic") : Array::make("drifter", "bloom");
        formation["count"] = std::min(18, 6 + int(std::floor(double(wave) * 0.55)));
        formation["spread_angle_deg"] = 52;
        formation["spiral_arms"] = 2 + (wave >= 21 ? 1 : 0);
    }

    Dictionary event;
    if (wave >= 7 && wave % 7 == 0) {
        event["type"] = "comet_cache";
        event["trigger_at_percent"] = 0.28;
        event["hp"] = 115.0 + double(wave) * 8.0;
        event["reward"] = 55 + wave * 5;
        event["duration"] = 18.0;
        event["expire_spawns"] = 3 + int(wave / 7);
    } else if (wave >= 9 && wave % 9 == 0) {
        event["type"] = "bio_lab_boost";
        event["multiplier"] = 2.0;
        event["duration"] = 24.0;
        event["trigger_at_percent"] = 0.0;
    } else if (wave >= 6 && wave % 6 == 0) {
        event["type"] = "mid_wave_autoflare";
        event["trigger_at_percent"] = 0.52;
        event["cryo_disruption_seconds"] = 5.0;
    } else if (wave >= 8 && wave % 8 == 0) {
        event["type"] = "ring_blind";
        event["rings"] = Array::make(int(wave / 8) % 4);
        event["duration"] = 12.0;
        event["trigger_at_percent"] = 0.18;
    }

    Dictionary data;
    data["index"] = wave;
    data["name"] = vformat("%s %02d", wave_name, wave);
    data["wave_type"] = wave_type;
    data["spawn_interval"] = interval;
    data["credit_reward"] = credit_reward;
    data["spawns"] = spawns;
    data["clash_groups"] = clash_groups;
    data["formation"] = formation;
    data["event"] = event;
    data["enemy_hp_scale"] = hp_scale;
    data["enemy_speed_scale"] = speed_scale;
    data["enemy_damage_scale"] = damage_scale;
    data["enemy_reward_scale"] = reward_scale;
    data["escalation_threshold_seconds"] = std::max(28.0, 44.0 - double(wave) * 0.35);
    data["tutorial_hint"] = "Endless mode: enemy mix, count, speed, and HP scale every wave. Clear one more.";
    return data;
}

Array GameWaveLibraryNative::repeat_variant(const String& variant, int count) const {
    Array values;
    for (int i = 0; i < std::max(0, count); ++i) {
        values.append(variant);
    }
    return values;
}

Variant GameWaveLibraryNative::daily_pick(const Array& pool, int wave, int salt) const {
    return pool[(daily_seed_value() + wave * 17 + salt * 31) % pool.size()];
}

Array GameWaveLibraryNative::spawn_entries(const Dictionary& wave_data) const {
    return array_value(wave_data.get("spawns", Array()));
}

Dictionary GameWaveLibraryNative::variant_counts(const Dictionary& wave_data) const {
    Dictionary counts;
    const String wave_type = String(wave_data.get("wave_type", "normal"));
    if (wave_type == "clash" || wave_type == "boss") {
        const Array groups = array_value(wave_data.get("clash_groups", Array()));
        for (int i = 0; i < groups.size(); ++i) {
            if (groups[i].get_type() != Variant::DICTIONARY) {
                continue;
            }
            Dictionary group = groups[i];
            const Array variants = array_value(group.get("variants", Array()));
            for (int j = 0; j < variants.size(); ++j) {
                add_variant_count(counts, variant_key(variants[j]), 1);
            }
        }
        if (!counts.is_empty()) {
            return counts;
        }
    }

    const Array entries = spawn_entries(wave_data);
    for (int i = 0; i < entries.size(); ++i) {
        if (entries[i].get_type() != Variant::DICTIONARY) {
            continue;
        }
        Dictionary entry = entries[i];
        add_variant_count(counts, variant_key(entry.get("variant", 0)), std::max(0, int_value(entry.get("count", 0))));
    }

    if (wave_type == "formation") {
        Variant formation_value = wave_data.get("formation", Dictionary());
        if (formation_value.get_type() == Variant::DICTIONARY) {
            Dictionary formation = formation_value;
            const Array formation_variants = array_value(formation.get("variants", Array::make("drifter")));
            const int formation_count = std::max(0, int_value(formation.get("count", 0)));
            if (!formation_variants.is_empty() && formation_count > 0) {
                for (int i = 0; i < formation_count; ++i) {
                    add_variant_count(counts, variant_key(formation_variants[i % formation_variants.size()]), 1);
                }
            }
        }
    }
    return counts;
}

void GameWaveLibraryNative::add_variant_count(Dictionary& counts, const String& variant, int amount) const {
    if (amount <= 0) {
        return;
    }
    counts[variant] = int_value(counts.get(variant, 0)) + amount;
}

String GameWaveLibraryNative::type_label(const Dictionary& wave_data) const {
    const String wave_type = String(wave_data.get("wave_type", "normal"));
    if (wave_type == "clash") return "CLASH";
    if (wave_type == "boss") return "BOSS";
    if (wave_type == "formation") return "FORMATION";
    return "STANDARD";
}

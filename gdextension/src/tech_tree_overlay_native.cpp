#include "tech_tree_overlay_native.h"

#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable.hpp>

#include <algorithm>

using namespace godot;

namespace {

constexpr double TREE_COLS = 4.0;
constexpr double TREE_ROWS = 6.0;
constexpr const char* TECH_TIER_ICON_PATH = "res://assets/ui/tech_tiers/%s_tier_%d.png";
constexpr const char* APEX_ICON_PATH = "res://assets/ui/tech_tiers/apex_tier.png";

struct UpgradeDef {
    const char* id;
    const char* title;
    int row;
    int col;
    const char* tower;
    int cost;
    const char* req;
    const char* desc;
};

constexpr UpgradeDef UPGRADE_DEFS[] = {
    {"solar_lens", "Solar Lens", 0, 0, "photon_splitter", 75, "", "Photon Splitters gain 10% range."},
    {"split_beam", "Split Beam", 0, 1, "photon_splitter", 180, "solar_lens", "Photon Splitters deal 12% more damage."},
    {"plasma_core", "Plasma Core", 0, 2, "photon_splitter", 360, "split_beam", "Photon Splitters fire 10% faster."},
    {"long_orbit", "Long Orbit", 1, 0, "cryo_probe", 90, "", "Cryo Probes gain 5% range."},
    {"far_sight", "Far Sight", 1, 1, "cryo_probe", 240, "long_orbit", "Cryo Probes gain 12% range and 25% longer slow."},
    {"brittle_shells", "Brittle Shells", 1, 2, "cryo_probe", 470, "far_sight", "Slowed enemies take 12% more non-control damage."},
    {"bio_splice", "Bio Splice", 2, 0, "bio_lab", 220, "", "Bio-Labs fire and excavate 12% faster."},
    {"salvage_culture", "Salvage Culture", 2, 1, "bio_lab", 410, "bio_splice", "Enemy kills pay 10% more Sol Credits."},
    {"solar_choir", "Solar Choir", 2, 2, "bio_lab", 1350, "salvage_culture", "Bio-Labs gain 10% damage and range."},
    {"rapid_charge", "Rapid Charge", 3, 0, "magnetic_net", 110, "", "Magnetic Nets pulse 5% faster."},
    {"magnetic_lattice", "Magnetic Lattice", 3, 1, "magnetic_net", 280, "rapid_charge", "Magnetic Nets gain 10% range."},
    {"gravitic_payload", "Gravitic Payload", 3, 2, "magnetic_net", 840, "magnetic_lattice", "Magnetic slows last 25% longer and Magnetic Nets gain 10% range."},
    {"stellar_lance", "Stellar Lance", 4, 0, "helios_cannon", 620, "", "Helios Cannons deal 14% more damage."},
    {"slingshot_coils", "Slingshot Coils", 4, 1, "helios_cannon", 950, "stellar_lance", "Helios Slingshot costs 30 Sol and hits 20% harder."},
    {"flare_battery", "Flare Battery", 4, 2, "helios_cannon", 760, "slingshot_coils", "Solar Flare charges every 2 cleared waves instead of 3."},
    {"pressure_hull", "Pressure Hull", 5, 0, "tardigrade_bomb", 90, "", "Tardigrade Bombs gain 10% range."},
    {"spore_nests", "Spore Nests", 5, 1, "tardigrade_bomb", 240, "pressure_hull", "Tardigrade Bombs fire 8% faster."},
    {"resilient_bloom", "Resilient Bloom", 5, 2, "tardigrade_bomb", 470, "spore_nests", "Tardigrade Bombs deal 14% more damage."},
    {"photon_apex", "Apex Photon", 0, 3, "photon_splitter", 900, "plasma_core", "Photon Splitters gain 8% damage, rate, and range."},
    {"cryo_apex", "Apex Cryo", 1, 3, "cryo_probe", 1000, "brittle_shells", "Cryo Probes gain 8% damage, rate, and range. Cryo slows last longer."},
    {"bio_apex", "Apex Bio-Lab", 2, 3, "bio_lab", 1500, "solar_choir", "Bio-Labs gain 8% damage, rate, and range."},
    {"magnetic_apex", "Apex Magnetic", 3, 3, "magnetic_net", 1200, "gravitic_payload", "Magnetic Nets gain 8% damage, rate, and range. Magnetic slows last longer."},
    {"helios_apex", "Apex Helios", 4, 3, "helios_cannon", 1600, "flare_battery", "Helios Cannons gain 8% damage, rate, and range. Solar Flare deals +10 damage."},
    {"tardigrade_apex", "Apex Tardigrade", 5, 3, "tardigrade_bomb", 1000, "resilient_bloom", "Tardigrade Bombs gain 8% damage, rate, and range."},
};

Color color_deep() { return Color(0.006, 0.012, 0.024, 0.95); }
Color color_cyan() { return Color(0.22, 0.84, 0.94, 0.88); }
Color color_gold() { return Color(1.0, 0.78, 0.26, 0.92); }
Color color_muted() { return Color(0.58, 0.70, 0.82, 0.86); }
Color color_text() { return Color(0.90, 0.96, 1.0, 1.0); }

Array apex_ids() {
    return Array::make("photon_apex", "cryo_apex", "bio_apex", "magnetic_apex", "helios_apex", "tardigrade_apex");
}

String tower_icon_path(const String& tower) {
    if (tower == "cryo_probe") return "res://assets/sprites/clean/towers/cryo_probe.png";
    if (tower == "bio_lab") return "res://assets/sprites/clean/towers/bio_lab.png";
    if (tower == "magnetic_net") return "res://assets/sprites/clean/towers/magnetic_net.png";
    if (tower == "helios_cannon") return "res://assets/sprites/clean/towers/helios_cannon.png";
    if (tower == "tardigrade_bomb") return "res://assets/sprites/clean/towers/tardigrade_bomb.png";
    return "res://assets/sprites/clean/towers/photon_splitter.png";
}

Dictionary upgrade_from_def(const UpgradeDef& def) {
    Dictionary upgrade;
    upgrade["id"] = def.id;
    upgrade["title"] = def.title;
    upgrade["row"] = def.row;
    upgrade["col"] = def.col;
    upgrade["tower"] = def.tower;
    upgrade["cost"] = def.cost;
    upgrade["req"] = String(def.req).is_empty() ? Array() : Array::make(def.req);
    upgrade["desc"] = def.desc;
    return upgrade;
}

Array upgrades() {
    Array values;
    for (const UpgradeDef& def : UPGRADE_DEFS) {
        values.append(upgrade_from_def(def));
    }
    return values;
}

Array links() {
    Array values;
    values.append(Array::make("solar_lens", "split_beam"));
    values.append(Array::make("split_beam", "plasma_core"));
    values.append(Array::make("plasma_core", "photon_apex"));
    values.append(Array::make("long_orbit", "far_sight"));
    values.append(Array::make("far_sight", "brittle_shells"));
    values.append(Array::make("brittle_shells", "cryo_apex"));
    values.append(Array::make("bio_splice", "salvage_culture"));
    values.append(Array::make("salvage_culture", "solar_choir"));
    values.append(Array::make("solar_choir", "bio_apex"));
    values.append(Array::make("rapid_charge", "magnetic_lattice"));
    values.append(Array::make("magnetic_lattice", "gravitic_payload"));
    values.append(Array::make("gravitic_payload", "magnetic_apex"));
    values.append(Array::make("stellar_lance", "slingshot_coils"));
    values.append(Array::make("slingshot_coils", "flare_battery"));
    values.append(Array::make("flare_battery", "helios_apex"));
    values.append(Array::make("pressure_hull", "spore_nests"));
    values.append(Array::make("spore_nests", "resilient_bloom"));
    values.append(Array::make("resilient_bloom", "tardigrade_apex"));
    return values;
}

double number(const Variant& value, double fallback = 0.0) {
    if (value.get_type() == Variant::INT || value.get_type() == Variant::FLOAT) {
        return static_cast<double>(value);
    }
    return fallback;
}

int number_int(const Variant& value, int fallback = 0) {
    if (value.get_type() == Variant::INT || value.get_type() == Variant::FLOAT) {
        return static_cast<int>(static_cast<int64_t>(value));
    }
    return fallback;
}

String pascal_node_name(const String& value) {
    String result;
    bool upper_next = true;
    for (int i = 0; i < value.length(); ++i) {
        const String ch = value.substr(i, 1);
        if (ch == "_" || ch == "-" || ch == " ") {
            upper_next = true;
            continue;
        }
        result += upper_next ? ch.to_upper() : ch;
        upper_next = false;
    }
    return result + "Node";
}

}

void TechTreeBoardNative::_bind_methods() {}

void TechTreeBoardNative::_draw() {
    for (int i = 0; i < links.size(); ++i) {
        if (links[i].get_type() != Variant::ARRAY) {
            continue;
        }
        const Array link = links[i];
        if (link.size() < 2) {
            continue;
        }
        const String from_id = String(link[0]);
        const String to_id = String(link[1]);
        if (!node_centers.has(from_id) || !node_centers.has(to_id)) {
            continue;
        }
        const String state_a = String(node_states.get(from_id, "locked"));
        const String state_b = String(node_states.get(to_id, "locked"));
        Color color(0.20, 0.84, 0.94, 0.46);
        if (state_a == "locked" || state_b == "locked") {
            color = Color(0.20, 0.32, 0.42, 0.48);
        } else if (state_a == "master" || state_b == "master") {
            color = Color(1.0, 0.78, 0.24, 0.62);
        }
        const Vector2 from_pos = node_centers[from_id];
        const Vector2 to_pos = node_centers[to_id];
        draw_line(from_pos, to_pos, Color(0.0, 0.0, 0.0, 0.52), 8.0, true);
        draw_line(from_pos, to_pos, color, 4.0, true);
        draw_circle(from_pos, 5.0, color);
        draw_circle(to_pos, 5.0, color);
    }
}

void TechTreeTooltipButtonNative::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_tooltip_data", "data"), &TechTreeTooltipButtonNative::set_tooltip_data);
    ClassDB::bind_method(D_METHOD("get_tooltip_data"), &TechTreeTooltipButtonNative::get_tooltip_data);
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "tooltip_data"), "set_tooltip_data", "get_tooltip_data");
}

Object* TechTreeTooltipButtonNative::_make_custom_tooltip(const String&) const {
    const Color accent = tooltip_data.get("accent", color_cyan());
    PanelContainer* panel = memnew(PanelContainer);
    panel->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
    panel->add_theme_stylebox_override("panel", tooltip_panel_style(accent));

    VBoxContainer* box = memnew(VBoxContainer);
    box->set_custom_minimum_size(Vector2(280.0, 0.0));
    box->add_theme_constant_override("separation", 6);
    panel->add_child(box);

    HBoxContainer* top = memnew(HBoxContainer);
    top->add_theme_constant_override("separation", 8);
    box->add_child(top);

    TextureRect* icon = memnew(TextureRect);
    icon->set_custom_minimum_size(Vector2(42.0, 42.0));
    icon->set_expand_mode(TextureRect::EXPAND_FIT_WIDTH_PROPORTIONAL);
    icon->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
    icon->set_texture(Ref<Texture2D>(Object::cast_to<Texture2D>(tooltip_data.get("icon", Variant()))));
    top->add_child(icon);

    VBoxContainer* title_box = memnew(VBoxContainer);
    title_box->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    title_box->add_theme_constant_override("separation", 2);
    top->add_child(title_box);

    Label* title = memnew(Label);
    title->set_text(String(tooltip_data.get("title", "")).to_upper());
    title->add_theme_font_size_override("font_size", 15);
    title->add_theme_color_override("font_color", Color(1.0, 0.78, 0.26, 0.96));
    title_box->add_child(title);

    Label* state = memnew(Label);
    state->set_text(String(tooltip_data.get("state", "")));
    state->add_theme_font_size_override("font_size", 10);
    state->add_theme_color_override("font_color", accent);
    title_box->add_child(state);

    Label* desc = memnew(Label);
    desc->set_text(String(tooltip_data.get("desc", "")));
    desc->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
    desc->add_theme_font_size_override("font_size", 12);
    desc->add_theme_color_override("font_color", Color(0.84, 0.91, 1.0, 0.94));
    box->add_child(desc);

    Label* req = memnew(Label);
    req->set_text(String(tooltip_data.get("req", "")));
    req->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
    req->add_theme_font_size_override("font_size", 11);
    req->add_theme_color_override("font_color", Color(0.58, 0.70, 0.82, 0.92));
    box->add_child(req);
    return panel;
}

void TechTreeTooltipButtonNative::set_tooltip_data(const Dictionary& data) { tooltip_data = data; }
Dictionary TechTreeTooltipButtonNative::get_tooltip_data() const { return tooltip_data; }

Ref<StyleBoxFlat> TechTreeTooltipButtonNative::tooltip_panel_style(const Color& border) const {
    Ref<StyleBoxFlat> style;
    style.instantiate();
    style->set_bg_color(Color(0.014, 0.020, 0.036, 0.98));
    style->set_border_color(border);
    style->set_border_width_all(1);
    style->set_corner_radius_all(8);
    style->set_content_margin(SIDE_LEFT, 12.0);
    style->set_content_margin(SIDE_RIGHT, 12.0);
    style->set_content_margin(SIDE_TOP, 10.0);
    style->set_content_margin(SIDE_BOTTOM, 10.0);
    style->set_shadow_color(Color(0.0, 0.0, 0.0, 0.50));
    style->set_shadow_size(9);
    style->set_shadow_offset(Vector2(0.0, 2.0));
    return style;
}

void TechTreeOverlayNative::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_layout_overlay"), &TechTreeOverlayNative::layout_overlay);
    ClassDB::bind_method(D_METHOD("_select_upgrade", "id"), &TechTreeOverlayNative::select_upgrade);
    ClassDB::bind_method(D_METHOD("_try_unlock_selected"), &TechTreeOverlayNative::try_unlock_selected);
    ClassDB::bind_method(D_METHOD("_toggle_tech_effects"), &TechTreeOverlayNative::toggle_tech_effects);
    ClassDB::bind_method(D_METHOD("_close"), &TechTreeOverlayNative::close_overlay);
}

void TechTreeOverlayNative::_ready() {
    set_process_mode(Node::PROCESS_MODE_ALWAYS);
    was_paused = get_tree() != nullptr && get_tree()->is_paused();
    if (get_tree() != nullptr) {
        get_tree()->set_pause(true);
    }
    upgrade_by_id.clear();
    const Array values = upgrades();
    for (int i = 0; i < values.size(); ++i) {
        const Dictionary upgrade = values[i];
        upgrade_by_id[String(upgrade["id"])] = upgrade;
    }
    build_overlay();
    select_upgrade(selected_id);
    refresh_tree();
    layout_overlay();
    if (get_viewport() != nullptr && !get_viewport()->is_connected("size_changed", Callable(this, "_layout_overlay"))) {
        get_viewport()->connect("size_changed", Callable(this, "_layout_overlay"));
    }
}

void TechTreeOverlayNative::_exit_tree() {
    if (get_tree() != nullptr) {
        get_tree()->set_pause(was_paused);
    }
}

void TechTreeOverlayNative::_unhandled_input(const Ref<InputEvent>& event) {
    Ref<InputEventKey> key_event = event;
    if (key_event.is_null() || !key_event->is_pressed() || key_event->is_echo()) {
        return;
    }
    const Key keycode = key_event->get_keycode();
    if (keycode == KEY_ESCAPE || keycode == KEY_T) {
        close_overlay();
        if (get_viewport() != nullptr) {
            get_viewport()->set_input_as_handled();
        }
    }
}

void TechTreeOverlayNative::build_overlay() {
    root = memnew(Control);
    root->set_name("OverlayRoot");
    root->set_mouse_filter(Control::MOUSE_FILTER_STOP);
    root->set_anchors_preset(Control::PRESET_FULL_RECT);
    add_child(root);

    ColorRect* shade = memnew(ColorRect);
    shade->set_name("Shade");
    shade->set_color(Color(0.004, 0.009, 0.020, 0.78));
    shade->set_mouse_filter(Control::MOUSE_FILTER_STOP);
    shade->set_anchors_preset(Control::PRESET_FULL_RECT);
    root->add_child(shade);

    main_panel = memnew(PanelContainer);
    main_panel->set_name("TechPanel");
    main_panel->set_mouse_filter(Control::MOUSE_FILTER_STOP);
    main_panel->add_theme_stylebox_override("panel", panel_style(color_deep(), color_cyan(), 8.0, 20.0, 16.0));
    root->add_child(main_panel);

    title_panel = memnew(PanelContainer);
    title_panel->set_name("TitlePanel");
    title_panel->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
    title_panel->add_theme_stylebox_override("panel", panel_style(Color(0.026, 0.060, 0.110, 0.98), color_cyan(), 8.0, 22.0, 14.0));
    root->add_child(title_panel);

    VBoxContainer* title_box = memnew(VBoxContainer);
    title_box->set_name("TitleBox");
    title_box->set_alignment(BoxContainer::ALIGNMENT_CENTER);
    title_box->add_theme_constant_override("separation", 5);
    title_panel->add_child(title_box);

    HBoxContainer* title_row = memnew(HBoxContainer);
    title_row->set_alignment(BoxContainer::ALIGNMENT_CENTER);
    title_row->add_theme_constant_override("separation", 16);
    title_box->add_child(title_row);

    title_label = memnew(Label);
    title_label->set_name("TitleLabel");
    title_label->set_text("SOL TECH TREE");
    title_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    title_label->add_theme_font_size_override("font_size", 31);
    title_row->add_child(title_label);

    xp_label = memnew(Label);
    xp_label->set_name("XpLabel");
    update_xp_label();
    xp_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    xp_label->add_theme_font_size_override("font_size", 24);
    xp_label->add_theme_color_override("font_color", Color(0.50, 1.0, 0.28, 1.0));
    title_row->add_child(xp_label);

    Label* subtitle_label = memnew(Label);
    subtitle_label->set_name("SubtitleLabel");
    subtitle_label->set_text("Each tower has its own path. Choose one apex.");
    subtitle_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    subtitle_label->add_theme_font_size_override("font_size", 15);
    subtitle_label->add_theme_color_override("font_color", Color(0.82, 0.90, 1.0, 0.94));
    title_box->add_child(subtitle_label);

    close_button = memnew(Button);
    close_button->set_name("BackButton");
    close_button->set_text("BACK");
    close_button->set_custom_minimum_size(Vector2(92, 58));
    close_button->connect("pressed", Callable(this, "_close"));
    root->add_child(close_button);

    effects_toggle_button = memnew(Button);
    effects_toggle_button->set_name("TechEffectsToggleButton");
    effects_toggle_button->set_custom_minimum_size(Vector2(176, 58));
    effects_toggle_button->set_tooltip_text("Toggle active tech upgrade effects.");
    effects_toggle_button->connect("pressed", Callable(this, "_toggle_tech_effects"));
    root->add_child(effects_toggle_button);
    refresh_effects_toggle_button();

    selected_panel = memnew(Panel);
    selected_panel->set_name("SelectedTechPanel");
    selected_panel->set_mouse_filter(Control::MOUSE_FILTER_STOP);
    selected_panel->add_theme_stylebox_override("panel", panel_style(Color(0.018, 0.022, 0.044, 0.97), color_gold(), 8.0, 18.0, 14.0));
    root->add_child(selected_panel);
    build_selected_panel();

    tree_board = memnew(TechTreeBoardNative);
    tree_board->set_name("TechTreeBoard");
    tree_board->set_mouse_filter(Control::MOUSE_FILTER_PASS);
    tree_board->links = links();
    root->add_child(tree_board);
    build_upgrade_nodes();

    Object* theme = space_theme();
    if (theme != nullptr) {
        theme->call("apply_fonts", root);
        theme->call("apply_secondary_button", close_button, theme->get("ICON_BACK_PATH"));
        theme->call("apply_secondary_button", effects_toggle_button);
        apply_xp_label_font();
    }
    close_button->add_theme_font_size_override("font_size", 15);
    effects_toggle_button->add_theme_font_size_override("font_size", 13);
    refresh_effects_toggle_button();
}

void TechTreeOverlayNative::build_selected_panel() {
    selected_box = memnew(VBoxContainer);
    selected_box->set_name("SelectedBox");
    selected_box->set_alignment(BoxContainer::ALIGNMENT_BEGIN);
    selected_box->add_theme_constant_override("separation", 8);
    selected_panel->add_child(selected_box);

    selected_title = memnew(Label);
    selected_title->set_name("SelectedTitle");
    selected_title->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    selected_title->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
    selected_title->add_theme_font_size_override("font_size", 19);
    selected_title->add_theme_color_override("font_color", color_gold());
    selected_box->add_child(selected_title);

    selected_icon = memnew(TextureRect);
    selected_icon->set_name("SelectedIcon");
    selected_icon->set_custom_minimum_size(Vector2(104, 104));
    selected_icon->set_expand_mode(TextureRect::EXPAND_FIT_WIDTH_PROPORTIONAL);
    selected_icon->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
    selected_box->add_child(selected_icon);

    selected_status = memnew(Label);
    selected_status->set_name("SelectedStatus");
    selected_status->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    selected_status->add_theme_font_size_override("font_size", 17);
    selected_box->add_child(selected_status);

    selected_cost = memnew(Label);
    selected_cost->set_name("SelectedCost");
    selected_cost->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    selected_cost->add_theme_font_size_override("font_size", 13);
    selected_box->add_child(selected_cost);

    selected_requires = memnew(Label);
    selected_requires->set_name("SelectedRequires");
    selected_requires->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    selected_requires->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
    selected_requires->add_theme_font_size_override("font_size", 11);
    selected_requires->add_theme_color_override("font_color", color_muted());
    selected_box->add_child(selected_requires);

    selected_desc = memnew(Label);
    selected_desc->set_name("SelectedDescription");
    selected_desc->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    selected_desc->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
    selected_desc->add_theme_font_size_override("font_size", 14);
    selected_desc->add_theme_color_override("font_color", Color(0.82, 0.90, 1.0, 0.92));
    selected_box->add_child(selected_desc);

    unlock_button = memnew(Button);
    unlock_button->set_name("UnlockButton");
    unlock_button->set_custom_minimum_size(Vector2(190, 48));
    unlock_button->connect("pressed", Callable(this, "_try_unlock_selected"));
    selected_box->add_child(unlock_button);
    if (Object* theme = space_theme()) {
        theme->call("apply_primary_button", unlock_button);
    }
    unlock_button->add_theme_font_size_override("font_size", 15);
}

void TechTreeOverlayNative::build_upgrade_nodes() {
    const Array values = upgrades();
    for (int i = 0; i < values.size(); ++i) {
        const Dictionary upgrade = values[i];
        const String id = upgrade["id"];
        const String state = upgrade_state(upgrade);
        tree_board->node_states[id] = state;

        Control* card = memnew(Control);
        card->set_name(pascal_node_name(id));
        card->set_mouse_filter(Control::MOUSE_FILTER_PASS);
        tree_board->add_child(card);
        node_cards[id] = card;

        TechTreeTooltipButtonNative* button = memnew(TechTreeTooltipButtonNative);
        button->set_name("IconButton");
        button->set_text("");
        button->set_focus_mode(Control::FOCUS_ALL);
        button->set_tooltip_text(id);
        button->tooltip_data = upgrade_tooltip_data(upgrade);
        button->set_expand_icon(true);
        button->set_button_icon(load_upgrade_icon(upgrade));
        button->connect("pressed", Callable(this, "_select_upgrade").bind(id));
        card->add_child(button);
        style_upgrade_button(button, state);

        Label* label = memnew(Label);
        label->set_name("NodeLabel");
        label->set_text(String(upgrade["title"]));
        label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
        label->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
        label->add_theme_font_size_override("font_size", 13);
        label->add_theme_color_override("font_color", state != "locked" ? color_text() : color_muted());
        card->add_child(label);

        Label* tag = memnew(Label);
        tag->set_name("NodeTag");
        tag->set_text(upgrade_tag(upgrade));
        tag->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
        tag->add_theme_font_size_override("font_size", 9);
        tag->add_theme_color_override("font_color", state_color(state));
        card->add_child(tag);
    }
}

void TechTreeOverlayNative::layout_overlay() {
    if (root == nullptr || get_viewport() == nullptr) {
        return;
    }
    const Vector2 viewport_size = get_viewport()->get_visible_rect().size;
    Vector2 panel_size(std::min(viewport_size.x - 48.0, 1480.0), std::min(viewport_size.y - 82.0, 840.0));
    panel_size.x = std::max(panel_size.x, 980.0f);
    panel_size.y = std::max(panel_size.y, 610.0f);
    Vector2 panel_pos = (viewport_size - panel_size) * 0.5 + Vector2(0.0, 18.0);
    panel_pos.x = std::max(20.0f, panel_pos.x);
    panel_pos.y = std::max(18.0f, panel_pos.y);
    main_panel->set_position(panel_pos);
    main_panel->set_size(panel_size);

    close_button->set_position(Vector2(panel_pos.x + 18.0, panel_pos.y + 16.0));
    close_button->set_size(Vector2(92.0, 58.0));
    effects_toggle_button->set_position(Vector2(panel_pos.x + panel_size.x - 196.0, panel_pos.y + 16.0));
    effects_toggle_button->set_size(Vector2(176.0, 58.0));

    const double title_width = std::min(static_cast<double>(panel_size.x - 460.0), 860.0);
    title_panel->set_position(Vector2(panel_pos.x + (panel_size.x - title_width) * 0.5, panel_pos.y + 14.0));
    title_panel->set_size(Vector2(title_width, 92.0));

    const double content_top = panel_pos.y + 132.0;
    const double content_bottom = panel_pos.y + panel_size.y - 30.0;
    const double content_h = content_bottom - content_top;
    const double gap = 20.0;
    const double left_w = Math::clamp(static_cast<double>(panel_size.x) * 0.24, 260.0, 320.0);
    const double tree_w = panel_size.x - left_w - gap * 3.0;
    double x = panel_pos.x + gap;

    selected_panel->set_position(Vector2(x, content_top));
    selected_panel->set_size(Vector2(left_w, content_h));
    if (selected_box != nullptr) {
        selected_box->set_position(Vector2(18.0, 18.0));
        selected_box->set_size(selected_panel->get_size() - Vector2(36.0, 36.0));
        selected_title->set_custom_minimum_size(Vector2(left_w - 48.0, 44.0));
        selected_desc->set_custom_minimum_size(Vector2(left_w - 48.0, 62.0));
        selected_requires->set_custom_minimum_size(Vector2(left_w - 48.0, 28.0));
    }
    x += left_w + gap;

    tree_board->set_position(Vector2(x, content_top + 18.0));
    tree_board->set_size(Vector2(tree_w, content_h - 36.0));
    layout_upgrade_nodes();
}

void TechTreeOverlayNative::layout_upgrade_nodes() {
    if (tree_board == nullptr) {
        return;
    }
    const Vector2 board_size = tree_board->get_size();
    const double lane_h = board_size.y / TREE_ROWS;
    const double card_h = std::max(62.0, lane_h - 4.0);
    const double node_size = std::min(Math::clamp(static_cast<double>(board_size.x) / 13.0, 48.0, 76.0), card_h - 38.0);
    const Vector2 card_size(std::max(node_size + 34.0, 104.0), card_h);
    Dictionary centers;
    const Array values = upgrades();
    for (int i = 0; i < values.size(); ++i) {
        const Dictionary upgrade = values[i];
        const String id = upgrade["id"];
        const double row = number(upgrade["row"]);
        const double col = number(upgrade["col"]);
        const Vector2 center((board_size.x / TREE_COLS) * (col + 0.5), (board_size.y / TREE_ROWS) * (row + 0.50));
        Control* card = Object::cast_to<Control>(node_cards[id]);
        if (card == nullptr) {
            continue;
        }
        card->set_position(center - card_size * 0.5);
        card->set_size(card_size);
        Button* button = Object::cast_to<Button>(card->get_node_or_null(NodePath("IconButton")));
        Label* label = Object::cast_to<Label>(card->get_node_or_null(NodePath("NodeLabel")));
        Label* tag = Object::cast_to<Label>(card->get_node_or_null(NodePath("NodeTag")));
        button->set_position(Vector2((card_size.x - node_size) * 0.5, 0.0));
        button->set_size(Vector2(node_size, node_size));
        label->set_position(Vector2(0.0, node_size + 4.0));
        label->set_size(Vector2(card_size.x, std::max(14.0, card_size.y - node_size - 18.0)));
        tag->set_position(Vector2(0.0, card_size.y - 14.0));
        tag->set_size(Vector2(card_size.x, 12.0));
        centers[id] = card->get_position() + button->get_position() + button->get_size() * 0.5;
    }
    tree_board->node_centers = centers;
    tree_board->queue_redraw();
}

void TechTreeOverlayNative::select_upgrade(const String& id) {
    if (!upgrade_by_id.has(id)) {
        return;
    }
    selected_id = id;
    const Dictionary selected_data = upgrade_by_id[id];
    const String state = upgrade_state(selected_data);
    selected_title->set_text(String(selected_data["title"]).to_upper());
    selected_icon->set_texture(load_upgrade_icon(selected_data));
    selected_status->set_text(upgrade_tag(selected_data));
    selected_status->add_theme_color_override("font_color", state_color(state));
    selected_cost->set_text(vformat("Research cost  %s XP", format_number(number_int(selected_data["cost"]))));
    selected_cost->add_theme_color_override("font_color", state != "locked" ? color_gold() : color_muted());
    selected_requires->set_text(requirement_text(selected_data));
    selected_desc->set_text(upgrade_description(selected_data));
    update_unlock_button(selected_data);

    const Array keys = node_cards.keys();
    for (int i = 0; i < keys.size(); ++i) {
        const String id_key = keys[i];
        Control* card = Object::cast_to<Control>(node_cards[id_key]);
        if (card == nullptr) {
            continue;
        }
        Button* button = Object::cast_to<Button>(card->get_node_or_null(NodePath("IconButton")));
        const String card_state = upgrade_state(upgrade_by_id[id_key]);
        style_upgrade_button(button, id_key == selected_id ? "selected" : card_state);
    }
}

void TechTreeOverlayNative::refresh_tree() {
    update_xp_label();
    const Array keys = node_cards.keys();
    for (int i = 0; i < keys.size(); ++i) {
        const String id = keys[i];
        const Dictionary upgrade = upgrade_by_id[id];
        const String state = upgrade_state(upgrade);
        tree_board->node_states[id] = state;
        Control* card = Object::cast_to<Control>(node_cards[id]);
        if (card == nullptr) {
            continue;
        }
        TechTreeTooltipButtonNative* button = Object::cast_to<TechTreeTooltipButtonNative>(card->get_node_or_null(NodePath("IconButton")));
        Label* label = Object::cast_to<Label>(card->get_node_or_null(NodePath("NodeLabel")));
        Label* tag = Object::cast_to<Label>(card->get_node_or_null(NodePath("NodeTag")));
        style_upgrade_button(button, id == selected_id ? "selected" : state);
        if (button != nullptr) {
            button->tooltip_data = upgrade_tooltip_data(upgrade);
        }
        label->add_theme_color_override("font_color", state != "locked" ? color_text() : color_muted());
        tag->set_text(upgrade_tag(upgrade));
        tag->add_theme_color_override("font_color", state_color(state));
    }
    tree_board->queue_redraw();
    select_upgrade(selected_id);
}

void TechTreeOverlayNative::try_unlock_selected() {
    if (!upgrade_by_id.has(selected_id)) {
        return;
    }
    const Dictionary upgrade = upgrade_by_id[selected_id];
    if (upgrade_state(upgrade) != "ready") {
        return;
    }
    if (Node* state = game_state()) {
        if (bool(state->call("unlock_tech", selected_id, number_int(upgrade["cost"]), requirements(upgrade)))) {
            refresh_tree();
        }
    }
}

void TechTreeOverlayNative::toggle_tech_effects() {
    Node* state = game_state();
    if (state == nullptr || !state->has_method("set_tech_effects_enabled")) {
        return;
    }
    state->call("set_tech_effects_enabled", !tech_effects_enabled());
    refresh_effects_toggle_button();
}

void TechTreeOverlayNative::refresh_effects_toggle_button() {
    if (effects_toggle_button == nullptr) {
        return;
    }
    const bool enabled = tech_effects_enabled();
    effects_toggle_button->set_text(enabled ? "EFFECTS\nON" : "EFFECTS\nOFF");
    effects_toggle_button->add_theme_color_override("font_color", enabled ? color_text() : color_muted());
    effects_toggle_button->add_theme_color_override("font_hover_color", color_text());
}

bool TechTreeOverlayNative::tech_effects_enabled() const {
    Node* state = game_state();
    if (state == nullptr || !state->has_method("get_tech_effects_enabled")) {
        return true;
    }
    return bool(state->call("get_tech_effects_enabled"));
}

void TechTreeOverlayNative::update_unlock_button(const Dictionary& upgrade) {
    const String state = upgrade_state(upgrade);
    const String id = upgrade["id"];
    const bool apex_blocked = is_apex_upgrade(id) && apex_choice_unlocked(id);
    const int cost = number_int(upgrade["cost"]);
    unlock_button->set_disabled(state != "ready");
    if (state == "unlocked" || state == "master") {
        unlock_button->set_text("UNLOCKED");
    } else if (state == "ready") {
        unlock_button->set_text(vformat("UNLOCK\n%s XP", format_number(cost)));
    } else if (requirements_met(upgrade)) {
        unlock_button->set_text(apex_blocked ? "APEX\nCHOSEN" : vformat("COST %s XP\nNEED %s MORE", format_number(cost), format_number(std::max(0, cost - tech_xp()))));
    } else {
        unlock_button->set_text("LOCKED");
    }
}

String TechTreeOverlayNative::upgrade_state(const Dictionary& upgrade) const {
    Node* state = game_state();
    const String id = upgrade.get("id", "");
    if (state != nullptr && bool(state->call("has_tech", id))) {
        return is_apex_upgrade(id) ? "master" : "unlocked";
    }
    if (is_apex_upgrade(id) && apex_choice_unlocked(id)) {
        return "locked";
    }
    if (!requirements_met(upgrade)) {
        return "locked";
    }
    return tech_xp() >= number_int(upgrade.get("cost", 0)) ? "ready" : "locked";
}

String TechTreeOverlayNative::upgrade_tag(const Dictionary& upgrade) const {
    const String state = upgrade_state(upgrade);
    if (state == "master") return "MASTER";
    if (state == "unlocked") return "UNLOCKED";
    if (state == "ready") return "READY";
    const String id = upgrade.get("id", "");
    if (is_apex_upgrade(id) && apex_choice_unlocked(id)) {
        return "CHOSEN";
    }
    return requirements_met(upgrade) ? vformat("%s XP", format_number(number_int(upgrade.get("cost", 0)))) : "LOCKED";
}

String TechTreeOverlayNative::requirement_text(const Dictionary& upgrade) const {
    const String id = upgrade.get("id", "");
    if (is_apex_upgrade(id) && apex_choice_unlocked(id)) {
        return "Apex already chosen";
    }
    const Array reqs = requirements(upgrade);
    if (reqs.is_empty()) {
        return "No prerequisite";
    }
    Array names;
    for (int i = 0; i < reqs.size(); ++i) {
        const String req_id = reqs[i];
        const Dictionary req_upgrade = upgrade_by_id.get(req_id, Dictionary());
        names.append(String(req_upgrade.get("title", req_id)));
    }
    return vformat("Requires  %s", String(" + ").join(names));
}

String TechTreeOverlayNative::upgrade_description(const Dictionary& upgrade) const {
    String desc = upgrade.get("desc", "");
    if (is_apex_upgrade(String(upgrade.get("id", "")))) {
        desc += " Only one apex can be chosen.";
    }
    return vformat("Tier %d - %s", upgrade_tier(upgrade), desc);
}

Dictionary TechTreeOverlayNative::upgrade_tooltip_data(const Dictionary& upgrade) const {
    const String state = upgrade_state(upgrade);
    Dictionary data;
    data["title"] = String(upgrade["title"]);
    data["state"] = upgrade_tag(upgrade);
    data["desc"] = upgrade_description(upgrade);
    data["req"] = requirement_text(upgrade);
    data["accent"] = state_color(state);
    data["icon"] = load_upgrade_icon(upgrade);
    return data;
}

bool TechTreeOverlayNative::requirements_met(const Dictionary& upgrade) const {
    Node* state = game_state();
    if (state == nullptr) {
        return false;
    }
    const Array reqs = requirements(upgrade);
    for (int i = 0; i < reqs.size(); ++i) {
        if (!bool(state->call("has_tech", String(reqs[i])))) {
            return false;
        }
    }
    return true;
}

Array TechTreeOverlayNative::requirements(const Dictionary& upgrade) const {
    const Variant req = upgrade.get("req", Array());
    return req.get_type() == Variant::ARRAY ? Array(req) : Array();
}

int TechTreeOverlayNative::upgrade_tier(const Dictionary& upgrade) const {
    return std::max(1, std::min(4, number_int(upgrade.get("col", 0)) + 1));
}

void TechTreeOverlayNative::close_overlay() {
    queue_free();
}

int TechTreeOverlayNative::tech_xp() const {
    Node* state = game_state();
    if (state == nullptr) {
        return 0;
    }
    return std::max(0, number_int(state->get("tech_xp")));
}

void TechTreeOverlayNative::update_xp_label() {
    if (xp_label != nullptr) {
        xp_label->set_text(vformat("XP BANK: %s", format_number(tech_xp())));
    }
}

void TechTreeOverlayNative::apply_xp_label_font() {
    Object* theme = space_theme();
    if (xp_label == nullptr || theme == nullptr) {
        return;
    }
    Ref<Resource> font_resource = ResourceLoader::get_singleton()->load(String(theme->get("FONT_BODY_PATH")));
    Ref<Font> font = font_resource;
    if (font.is_valid()) {
        xp_label->add_theme_font_override("font", font);
    }
}

String TechTreeOverlayNative::format_number(int value) const {
    const String text = String::num_int64(value);
    String out;
    int count = 0;
    for (int i = text.length() - 1; i >= 0; --i) {
        if (count > 0 && count % 3 == 0) {
            out = "," + out;
        }
        out = text.substr(i, 1) + out;
        ++count;
    }
    return out;
}

Ref<Texture2D> TechTreeOverlayNative::load_upgrade_icon(const Dictionary& upgrade) const {
    const String tower = upgrade.get("tower", "photon_splitter");
    const String fallback = tower_icon_path(tower);
    if (is_apex_upgrade(String(upgrade.get("id", "")))) {
        return load_texture(APEX_ICON_PATH, fallback);
    }
    return load_texture(vformat(TECH_TIER_ICON_PATH, tower, upgrade_tier(upgrade)), fallback);
}

bool TechTreeOverlayNative::is_apex_upgrade(const String& id) const {
    return apex_ids().has(id);
}

bool TechTreeOverlayNative::apex_choice_unlocked(const String& except_id) const {
    Node* state = game_state();
    if (state == nullptr) {
        return false;
    }
    String except = is_apex_upgrade(except_id) ? except_id : "";
    const Array ids = apex_ids();
    for (int i = 0; i < ids.size(); ++i) {
        const String id = ids[i];
        if (id != except && bool(state->call("has_tech", id))) {
            return true;
        }
    }
    return false;
}

Ref<Texture2D> TechTreeOverlayNative::load_texture(const String& path, const String& fallback_path) const {
    Ref<Texture2D> texture = ResourceLoader::get_singleton()->load(path);
    if (texture.is_valid()) {
        return texture;
    }
    if (!fallback_path.is_empty() && fallback_path != path) {
        return load_texture(fallback_path);
    }
    return Ref<Texture2D>();
}

Color TechTreeOverlayNative::state_color(const String& state) const {
    if (state == "master" || state == "selected") return color_gold();
    if (state == "unlocked") return Color(0.48, 1.0, 0.64, 0.98);
    if (state == "ready") return color_cyan();
    return color_muted();
}

void TechTreeOverlayNative::style_upgrade_button(Button* button, const String& state) const {
    if (button == nullptr) {
        return;
    }
    const Color accent = state_color(state);
    Color bg(0.020, 0.052, 0.078, 0.98);
    int border_width = 1;
    if (state == "locked") {
        bg = Color(0.014, 0.020, 0.032, 0.90);
    } else if (state == "selected") {
        bg = Color(0.080, 0.068, 0.026, 0.98);
        border_width = 3;
    } else if (state == "master") {
        bg = Color(0.078, 0.056, 0.014, 0.98);
        border_width = 2;
    }
    button->add_theme_stylebox_override("normal", button_style(bg, accent, 42.0, border_width));
    button->add_theme_stylebox_override("hover", button_style(Color(bg.r + 0.020, bg.g + 0.030, bg.b + 0.040, 1.0), Color(accent.r, accent.g, accent.b, 1.0), 42.0, std::max(2, border_width)));
    button->add_theme_stylebox_override("pressed", button_style(Color(0.052, 0.112, 0.138, 1.0), color_gold(), 42.0, 2));
    button->add_theme_stylebox_override("focus", button_style(Color(0.015, 0.072, 0.092, 1.0), color_gold(), 42.0, 2));
    button->add_theme_color_override("font_color", color_text());
    button->add_theme_color_override("font_hover_color", color_text());
    button->add_theme_color_override("font_pressed_color", color_text());
    button->add_theme_color_override("font_focus_color", color_text());
    button->add_theme_constant_override("h_separation", 5);
    button->set_default_cursor_shape(Control::CURSOR_POINTING_HAND);
    button->set_modulate(state == "locked" ? Color(1.0, 1.0, 1.0, 0.72) : Color(1.0, 1.0, 1.0, 1.0));
}

Ref<StyleBoxFlat> TechTreeOverlayNative::panel_style(const Color& bg, const Color& border, double radius, double margin_x, double margin_y) const {
    Ref<StyleBoxFlat> style;
    style.instantiate();
    style->set_bg_color(bg);
    style->set_border_color(border);
    style->set_border_width_all(1);
    style->set_corner_radius_all(int(radius));
    style->set_content_margin(SIDE_LEFT, margin_x);
    style->set_content_margin(SIDE_RIGHT, margin_x);
    style->set_content_margin(SIDE_TOP, margin_y);
    style->set_content_margin(SIDE_BOTTOM, margin_y);
    style->set_shadow_color(Color(0.0, 0.0, 0.0, 0.46));
    style->set_shadow_size(9);
    style->set_shadow_offset(Vector2(0.0, 2.0));
    return style;
}

Ref<StyleBoxFlat> TechTreeOverlayNative::button_style(const Color& bg, const Color& border, double radius, int border_width) const {
    Ref<StyleBoxFlat> style = panel_style(bg, border, radius, 8.0, 8.0);
    style->set_border_width_all(border_width);
    style->set_shadow_size(6);
    return style;
}

Object* TechTreeOverlayNative::space_theme() const {
    static Ref<RefCounted> resource;
    if (resource.is_null()) {
        resource = Ref<RefCounted>(Object::cast_to<RefCounted>(ClassDB::instantiate("SpaceThemeNative")));
    }
    return resource.ptr();
}

Node* TechTreeOverlayNative::game_state() const {
    return get_node_or_null(NodePath("/root/GameState"));
}

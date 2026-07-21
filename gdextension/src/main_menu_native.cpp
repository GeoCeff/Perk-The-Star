#include "main_menu_native.h"

#include <godot_cpp/classes/box_container.hpp>
#include <godot_cpp/classes/atlas_texture.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/grid_container.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/panel_container.hpp>
#include <godot_cpp/classes/progress_bar.hpp>
#include <godot_cpp/classes/property_tweener.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scroll_container.hpp>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/tween.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace {

constexpr const char* RUN_MODE_META = "run_mode";
constexpr const char* GAME_SCENE_PATH = "res://scenes/game.tscn";
constexpr const char* TECH_TREE_OVERLAY_SCENE_PATH = "res://scenes/ui/tech_tree_overlay.tscn";
constexpr const char* ACHIEVEMENT_EMBLEMS_PATH = "res://assets/ui/icons/achievement_emblems.svg";
constexpr const char* GAME_TITLE = "PERK THE STAR";
constexpr const char* SUBTITLE = "DEFEND THE SUN - SAVE THE SYSTEM";
constexpr const char* TAGLINE = "Defend me, defend me! - Oa ka Perk!";
constexpr const char* OVERVIEW = "Command orbiting towers around the Sun, survive the Prime campaign, brave Endless, or clear No-Flare Challenge, then bank Tech XP into six tower paths.";
constexpr const char* TECH_LABEL = "CMSC 21 | C++ / Godot Engine 4.x / GDExtension";
constexpr const char* AUTHOR_LABEL = "Geo Ceff Gabaisen & Dexter Juevesano";

struct ModeInfo {
    const char* key;
    const char* title;
    const char* icon;
    const char* kicker;
    const char* body;
    const char* note;
};

constexpr ModeInfo MODE_INFO[] = {
    {"campaign", "Normal Defense", "res://assets/ui/icons/icon_mode_campaign.png", "12-WAVE PRIME CAMPAIGN", "Classic defense run. Build a stable orbit, answer each enemy type, and survive the Prime finale.", "Best for steady Tech XP and learning tower paths."},
    {"boss_rush", "Boss Rush", "res://assets/ui/icons/icon_mode_boss_rush.png", "COMPRESSED PRIME PRESSURE", "Four heavy waves built around Prime-class threats. Less ramp-up, more immediate crisis control.", "Bio-Lab shell cracking matters early."},
    {"daily_seed", "Daily Seed", "res://assets/ui/icons/icon_mode_daily_seed.png", "FIXED DAILY CHALLENGE", "A six-wave formation locked to today's seed. Everyone gets the same enemy mix for cleaner replays.", "Good for routing and score chasing."},
    {"draft_defense", "Draft Defense", "res://assets/ui/icons/icon_mode_draft_defense.png", "CONTRACT BEFORE WAVE 1", "Pick a package before the first wave, then play around its tower bonuses and restrictions.", "Stronger identity, less autopilot."},
    {"endless", "Endless Defense", "res://assets/ui/icons/icon_mode_endless.png", "SCALING SURVIVAL", "Enemy count, speed, and health keep climbing after the normal campaign limit.", "Play for one more wave."},
    {"no_flare", "No-Flare Challenge", "res://assets/ui/icons/icon_mode_no_flare.png", "NO SOLAR FLARE SAFETY NET", "Clear the Prime mission without Solar Flare. Your tower placement has to solve every breach.", "Higher pressure, bonus Tech XP."},
};

Node* singleton(Node* owner, const String& name) {
    return owner->get_node_or_null(NodePath(String("/root/") + name));
}

void set_label_text(Label* label, const String& text) {
    if (label != nullptr) {
        label->set_text(text);
    }
}

const ModeInfo* mode_info(const String& mode) {
    for (const ModeInfo& info : MODE_INFO) {
        if (mode == info.key) {
            return &info;
        }
    }
    return nullptr;
}

}

void MainMenuNative::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_show_mode_overlay"), &MainMenuNative::show_mode_overlay);
    ClassDB::bind_method(D_METHOD("_show_mode_info", "mode"), &MainMenuNative::show_mode_info);
    ClassDB::bind_method(D_METHOD("_close_mode_overlay"), &MainMenuNative::close_mode_overlay);
    ClassDB::bind_method(D_METHOD("_show_tech_tree"), &MainMenuNative::show_tech_tree);
    ClassDB::bind_method(D_METHOD("_show_achievements"), &MainMenuNative::show_achievements);
    ClassDB::bind_method(D_METHOD("_close_achievements"), &MainMenuNative::close_achievements);
    ClassDB::bind_method(D_METHOD("_start_mode", "mode"), &MainMenuNative::start_mode);
}

void MainMenuNative::_ready() {
    btn_play = Object::cast_to<Button>(get_node_or_null(NodePath("CenterContainer/menu_box/button_box/btn_play")));
    btn_tech_tree = Object::cast_to<Button>(get_node_or_null(NodePath("CenterContainer/menu_box/button_box/btn_tech_tree")));
    btn_achievements = Object::cast_to<Button>(get_node_or_null(NodePath("CenterContainer/menu_box/button_box/btn_achievements")));
    btn_codex = Object::cast_to<Button>(get_node_or_null(NodePath("CenterContainer/menu_box/button_box/btn_codex")));
    btn_settings = Object::cast_to<Button>(get_node_or_null(NodePath("CenterContainer/menu_box/button_box/btn_settings")));
    btn_credits = Object::cast_to<Button>(get_node_or_null(NodePath("CenterContainer/menu_box/button_box/btn_credits")));
    btn_exit = Object::cast_to<Button>(get_node_or_null(NodePath("CenterContainer/menu_box/button_box/btn_exit")));
    menu_frame = Object::cast_to<PanelContainer>(get_node_or_null(NodePath("menu_frame")));
    title_label = Object::cast_to<Label>(get_node_or_null(NodePath("CenterContainer/menu_box/title_label")));
    sub_label = Object::cast_to<Label>(get_node_or_null(NodePath("CenterContainer/menu_box/sub_label")));
    tagline_label = Object::cast_to<Label>(get_node_or_null(NodePath("CenterContainer/menu_box/tagline_label")));
    description_label = Object::cast_to<Label>(get_node_or_null(NodePath("CenterContainer/menu_box/description_label")));
    version_label = Object::cast_to<Label>(get_node_or_null(NodePath("version_label")));
    author_label = Object::cast_to<Label>(get_node_or_null(NodePath("author_label")));

    if (Node* state = singleton(this, "GameState")) {
        state->call("reset_state");
        state->call("load_audio_settings");
        state->call("ensure_music_audible");
    }
    if (Node* music = singleton(this, "MusicManager")) {
        music->call("play_menu_music");
    }

    set_label_text(title_label, GAME_TITLE);
    set_label_text(sub_label, SUBTITLE);
    set_label_text(tagline_label, TAGLINE);
    set_label_text(description_label, OVERVIEW);
    set_label_text(version_label, TECH_LABEL);
    set_label_text(author_label, AUTHOR_LABEL);

    apply_menu_style();

    if (btn_play != nullptr) {
        btn_play->connect("pressed", Callable(this, "_show_mode_overlay"));
    }
    if (btn_tech_tree != nullptr) {
        btn_tech_tree->connect("pressed", Callable(this, "_show_tech_tree"));
    }
    if (btn_achievements != nullptr) {
        btn_achievements->connect("pressed", Callable(this, "_show_achievements"));
    }
    if (title_label != nullptr) {
        Color modulate = title_label->get_modulate();
        modulate.a = 0.0f;
        title_label->set_modulate(modulate);
        Ref<Tween> tween = create_tween();
        tween->tween_property(title_label, "modulate:a", 1.0, 1.5);
    }
    if (btn_play != nullptr) {
        btn_play->grab_focus();
    }
}

void MainMenuNative::_input(const Ref<InputEvent>& event) {
    if (event.is_valid() && event->is_action_pressed("ui_accept")) {
        Control* focus_owner = get_viewport() != nullptr ? Object::cast_to<Control>(get_viewport()->gui_get_focus_owner()) : nullptr;
        if (focus_owner == nullptr && btn_play != nullptr) {
            btn_play->emit_signal("pressed");
        }
    }
}

void MainMenuNative::_unhandled_input(const Ref<InputEvent>& event) {
    if (achievements_overlay != nullptr && event.is_valid() && event->is_action_pressed("ui_cancel")) {
        close_achievements();
        if (get_viewport() != nullptr) get_viewport()->set_input_as_handled();
        return;
    }
    if (mode_overlay != nullptr && event.is_valid() && event->is_action_pressed("ui_cancel")) {
        close_mode_overlay();
        if (get_viewport() != nullptr) {
            get_viewport()->set_input_as_handled();
        }
    }
}

void MainMenuNative::apply_menu_style() {
    Object* theme = space_theme();
    if (theme == nullptr) {
        return;
    }

    theme->call("apply_cursor");
    theme->call("apply_fonts", this);
    theme->call("apply_deep_panel", menu_frame, theme->get("COLOR_CYAN"));

    if (title_label != nullptr) title_label->add_theme_color_override("font_color", Color(1.0, 0.88, 0.36));
    if (sub_label != nullptr) sub_label->add_theme_color_override("font_color", Color(0.90, 0.94, 1.0));
    if (tagline_label != nullptr) tagline_label->add_theme_color_override("font_color", Color(0.55, 0.84, 0.92));
    if (description_label != nullptr) description_label->add_theme_color_override("font_color", Color(0.90, 0.94, 1.0));
    if (version_label != nullptr) version_label->add_theme_color_override("font_color", Color(0.78, 0.90, 1.0, 1.0));
    if (author_label != nullptr) author_label->add_theme_color_override("font_color", Color(0.68, 0.94, 1.0, 1.0));

    theme->call("apply_primary_button", btn_play, theme->get("ICON_PLAY_PATH"));
    theme->call("apply_secondary_button", btn_tech_tree, theme->get("ICON_TECH_TREE_PATH"));
    theme->call("apply_secondary_button", btn_achievements);
    if (btn_achievements != nullptr) btn_achievements->set_button_icon(achievement_icon(4));
    theme->call("apply_secondary_button", btn_codex, theme->get("ICON_CODEX_PATH"));
    theme->call("apply_secondary_button", btn_settings, theme->get("ICON_SETTINGS_PATH"));
    theme->call("apply_secondary_button", btn_credits, theme->get("ICON_CREDITS_PATH"));
    theme->call("apply_danger_button", btn_exit, theme->get("ICON_BACK_PATH"));
    if (btn_tech_tree != nullptr) {
        btn_tech_tree->add_theme_font_size_override("font_size", 20);
    }

    Array buttons = get_tree()->get_nodes_in_group("main_menu_buttons");
    for (int i = 0; i < buttons.size(); ++i) {
        if (Button* button = Object::cast_to<Button>(buttons[i])) {
            button->add_theme_font_size_override("font_size", 20);
        }
    }
}

void MainMenuNative::show_mode_overlay() {
    if (mode_overlay != nullptr) {
        return;
    }
    mode_overlay = memnew(Control);
    mode_overlay->set_name("ModeOverlay");
    mode_overlay->set_mouse_filter(Control::MOUSE_FILTER_STOP);
    mode_overlay->set_anchors_preset(Control::PRESET_FULL_RECT);
    add_child(mode_overlay);

    ColorRect* shade = memnew(ColorRect);
    shade->set_name("Shade");
    shade->set_color(Color(0.0, 0.0, 0.0, 0.62));
    shade->set_mouse_filter(Control::MOUSE_FILTER_STOP);
    shade->set_anchors_preset(Control::PRESET_FULL_RECT);
    mode_overlay->add_child(shade);

    PanelContainer* panel = memnew(PanelContainer);
    panel->set_name("ModePanel");
    panel->set_custom_minimum_size(Vector2(560, 570));
    panel->set_anchors_preset(Control::PRESET_CENTER);
    panel->set_offset(SIDE_LEFT, -280.0);
    panel->set_offset(SIDE_TOP, -285.0);
    panel->set_offset(SIDE_RIGHT, 280.0);
    panel->set_offset(SIDE_BOTTOM, 285.0);
    mode_overlay->add_child(panel);
    build_mode_info_panel();

    VBoxContainer* box = memnew(VBoxContainer);
    box->set_name("ModeBox");
    box->set_alignment(BoxContainer::ALIGNMENT_CENTER);
    box->add_theme_constant_override("separation", 12);
    panel->add_child(box);

    Label* title = memnew(Label);
    title->set_text("SELECT DEFENSE MODE");
    title->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    title->add_theme_font_size_override("font_size", 26);
    box->add_child(title);

    Label* subtitle = memnew(Label);
    subtitle->set_text("Choose the run type before launch.");
    subtitle->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    subtitle->add_theme_font_size_override("font_size", 14);
    subtitle->add_theme_color_override("font_color", Color(0.76, 0.88, 0.96, 0.92));
    box->add_child(subtitle);

    add_mode_button(box, "campaign", true);
    add_mode_button(box, "boss_rush", false);
    add_mode_button(box, "daily_seed", false);
    add_mode_button(box, "draft_defense", false);
    add_mode_button(box, "endless", false);
    add_mode_button(box, "no_flare", false);

    Button* back = memnew(Button);
    back->set_text("BACK");
    back->set_custom_minimum_size(Vector2(360, 48));
    back->connect("pressed", Callable(this, "_close_mode_overlay"));
    box->add_child(back);

    Object* theme = space_theme();
    if (theme != nullptr) {
        theme->call("apply_fonts", mode_overlay);
        theme->call("apply_deep_panel", panel, theme->get("COLOR_CYAN"));
        theme->call("apply_deep_panel", mode_info_panel, theme->get("COLOR_GOLD"));
        theme->call("apply_secondary_button", back);
        title->add_theme_color_override("font_color", theme->get("COLOR_GOLD"));
    }
    if (Button* first_button = Object::cast_to<Button>(box->get_child(2))) {
        first_button->grab_focus();
    }
}

void MainMenuNative::add_mode_button(VBoxContainer* box, const String& mode, bool primary) {
    const ModeInfo* info = mode_info(mode);
    if (box == nullptr || info == nullptr) {
        return;
    }
    Button* button = memnew(Button);
    button->set_text(info->title);
    button->set_custom_minimum_size(Vector2(360, 52));
    button->connect("pressed", Callable(this, "_start_mode").bind(mode));
    button->connect("mouse_entered", Callable(this, "_show_mode_info").bind(mode));
    button->connect("focus_entered", Callable(this, "_show_mode_info").bind(mode));
    box->add_child(button);

    Object* theme = space_theme();
    if (theme != nullptr) {
        theme->call(primary ? "apply_primary_button" : "apply_secondary_button", button, info->icon);
    }
}

void MainMenuNative::build_mode_info_panel() {
    mode_info_panel = memnew(PanelContainer);
    mode_info_panel->set_name("ModeInfoPanel");
    mode_info_panel->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
    mode_info_panel->set_custom_minimum_size(Vector2(350, 0));
    mode_info_panel->set_anchors_preset(Control::PRESET_CENTER);
    mode_info_panel->set_offset(SIDE_LEFT, 310.0);
    mode_info_panel->set_offset(SIDE_TOP, -215.0);
    mode_info_panel->set_offset(SIDE_RIGHT, 660.0);
    mode_info_panel->set_offset(SIDE_BOTTOM, 80.0);
    mode_info_panel->set_visible(false);
    mode_overlay->add_child(mode_info_panel);

    VBoxContainer* box = memnew(VBoxContainer);
    box->set_name("ModeInfoBox");
    box->add_theme_constant_override("separation", 8);
    mode_info_panel->add_child(box);

    mode_info_kicker = memnew(Label);
    mode_info_kicker->add_theme_font_size_override("font_size", 11);
    mode_info_kicker->add_theme_color_override("font_color", Color(0.22, 0.84, 0.94, 0.92));
    box->add_child(mode_info_kicker);

    mode_info_title = memnew(Label);
    mode_info_title->add_theme_font_size_override("font_size", 21);
    mode_info_title->add_theme_color_override("font_color", Color(1.0, 0.78, 0.26, 0.96));
    box->add_child(mode_info_title);

    mode_info_body = memnew(Label);
    mode_info_body->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
    mode_info_body->add_theme_font_size_override("font_size", 14);
    mode_info_body->add_theme_color_override("font_color", Color(0.88, 0.94, 1.0, 0.96));
    box->add_child(mode_info_body);

    mode_info_note = memnew(Label);
    mode_info_note->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
    mode_info_note->add_theme_font_size_override("font_size", 12);
    mode_info_note->add_theme_color_override("font_color", Color(0.62, 0.74, 0.86, 0.92));
    box->add_child(mode_info_note);
}

void MainMenuNative::show_mode_info(const String& mode) {
    if (mode_info_panel == nullptr) {
        return;
    }
    const ModeInfo* info = mode_info(mode);
    if (info == nullptr) {
        mode_info_panel->set_visible(false);
        return;
    }
    mode_info_kicker->set_text(String(info->kicker).to_upper());
    mode_info_title->set_text(String(info->title).to_upper());
    mode_info_body->set_text(info->body);
    mode_info_note->set_text(info->note);
    mode_info_panel->set_visible(true);
}

void MainMenuNative::close_mode_overlay() {
    if (mode_overlay != nullptr) {
        mode_overlay->queue_free();
    }
    mode_overlay = nullptr;
    mode_info_panel = nullptr;
    mode_info_title = nullptr;
    mode_info_kicker = nullptr;
    mode_info_body = nullptr;
    mode_info_note = nullptr;
}

void MainMenuNative::show_tech_tree() {
    if (get_node_or_null(NodePath("TechTreeOverlay")) != nullptr) {
        return;
    }
    Ref<Resource> resource = ResourceLoader::get_singleton()->load(TECH_TREE_OVERLAY_SCENE_PATH);
    Ref<PackedScene> packed = resource;
    if (packed.is_null()) {
        UtilityFunctions::push_error("Main menu could not load tech tree overlay.");
        return;
    }
    add_child(packed->instantiate());
}

void MainMenuNative::show_achievements() {
    if (achievements_overlay != nullptr) return;
    Node* state = singleton(this, "GameState");
    if (state == nullptr || !state->has_method("achievement_board")) return;
    const Array achievements = state->call("achievement_board");

    achievements_overlay = memnew(Control);
    achievements_overlay->set_name("AchievementsOverlay");
    achievements_overlay->set_mouse_filter(Control::MOUSE_FILTER_STOP);
    achievements_overlay->set_anchors_preset(Control::PRESET_FULL_RECT);
    add_child(achievements_overlay);

    ColorRect* shade = memnew(ColorRect);
    shade->set_color(Color(0.0, 0.0, 0.0, 0.76));
    shade->set_mouse_filter(Control::MOUSE_FILTER_STOP);
    shade->set_anchors_preset(Control::PRESET_FULL_RECT);
    achievements_overlay->add_child(shade);

    PanelContainer* panel = memnew(PanelContainer);
    panel->set_name("AchievementsBoard");
    panel->set_custom_minimum_size(Vector2(1100, 680));
    panel->set_anchors_preset(Control::PRESET_CENTER);
    panel->set_offset(SIDE_LEFT, -550.0);
    panel->set_offset(SIDE_TOP, -340.0);
    panel->set_offset(SIDE_RIGHT, 550.0);
    panel->set_offset(SIDE_BOTTOM, 340.0);
    achievements_overlay->add_child(panel);

    VBoxContainer* root = memnew(VBoxContainer);
    root->set_name("BoardContent");
    root->add_theme_constant_override("separation", 10);
    panel->add_child(root);

    Label* title = memnew(Label);
    title->set_text("CONSTELLATION OF DEEDS");
    title->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    title->add_theme_font_size_override("font_size", 30);
    title->add_theme_color_override("font_color", Color(1.0, 0.84, 0.30));
    root->add_child(title);

    int unlocked_count = 0;
    for (int i = 0; i < achievements.size(); ++i) {
        if (bool(Dictionary(achievements[i]).get("unlocked", false))) ++unlocked_count;
    }
    Label* summary = memnew(Label);
    summary->set_text(vformat("%d / %d EMBLEMS LIT  -  Finished runs update progress automatically.", unlocked_count, achievements.size()));
    summary->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    summary->add_theme_font_size_override("font_size", 14);
    summary->add_theme_color_override("font_color", Color(0.66, 0.86, 0.96, 0.92));
    root->add_child(summary);

    ScrollContainer* scroll = memnew(ScrollContainer);
    scroll->set_name("AchievementScroll");
    scroll->set_custom_minimum_size(Vector2(1040, 520));
    scroll->set_horizontal_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED);
    scroll->set_vertical_scroll_mode(ScrollContainer::SCROLL_MODE_AUTO);
    scroll->set_focus_mode(Control::FOCUS_ALL);
    scroll->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    scroll->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    root->add_child(scroll);

    GridContainer* grid = memnew(GridContainer);
    grid->set_name("AchievementGrid");
    grid->set_columns(3);
    grid->add_theme_constant_override("h_separation", 10);
    grid->add_theme_constant_override("v_separation", 10);
    grid->set_h_size_flags(Control::SIZE_EXPAND_FILL);
    scroll->add_child(grid);

    Object* theme = space_theme();
    for (int i = 0; i < achievements.size(); ++i) {
        const Dictionary item = achievements[i];
        const bool unlocked = bool(item.get("unlocked", false));
        const int current = int(item.get("current", 0));
        const int target = MAX(1, int(item.get("target", 1)));
        const Color accent = unlocked ? Color(item.get("accent", Color(0.22, 0.84, 0.94))) : Color(0.26, 0.34, 0.42);

        PanelContainer* card = memnew(PanelContainer);
        card->set_custom_minimum_size(Vector2(335, 154));
        grid->add_child(card);
        if (theme != nullptr) theme->call("apply_deep_panel", card, accent);

        HBoxContainer* row = memnew(HBoxContainer);
        row->add_theme_constant_override("separation", 10);
        card->add_child(row);

        TextureRect* emblem = memnew(TextureRect);
        emblem->set_custom_minimum_size(Vector2(68, 68));
        emblem->set_texture(achievement_icon(int(item.get("icon_index", i))));
        emblem->set_expand_mode(TextureRect::EXPAND_IGNORE_SIZE);
        emblem->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
        emblem->set_modulate(unlocked ? Color(1, 1, 1, 1) : Color(0.38, 0.44, 0.50, 0.78));
        row->add_child(emblem);

        VBoxContainer* text_box = memnew(VBoxContainer);
        text_box->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        text_box->add_theme_constant_override("separation", 3);
        row->add_child(text_box);

        Label* status = memnew(Label);
        status->set_text(unlocked ? "EMBLEM LIT" : vformat("CHALLENGE  %d / %d", current, target));
        status->add_theme_font_size_override("font_size", 10);
        status->add_theme_color_override("font_color", accent);
        text_box->add_child(status);

        Label* name = memnew(Label);
        name->set_text(String(item.get("title", "Unknown deed")).to_upper());
        name->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
        name->add_theme_font_size_override("font_size", 16);
        name->add_theme_color_override("font_color", unlocked ? Color(0.96, 0.99, 1.0) : Color(0.68, 0.74, 0.80));
        text_box->add_child(name);

        Label* description = memnew(Label);
        description->set_text(item.get("description", ""));
        description->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
        description->add_theme_font_size_override("font_size", 12);
        description->add_theme_color_override("font_color", unlocked ? Color(0.78, 0.88, 0.94) : Color(0.50, 0.58, 0.66));
        text_box->add_child(description);

        ProgressBar* progress = memnew(ProgressBar);
        progress->set_custom_minimum_size(Vector2(0, 8));
        progress->set_min(0);
        progress->set_max(target);
        progress->set_value(current);
        progress->set_show_percentage(false);
        if (theme != nullptr) {
            progress->add_theme_stylebox_override("background", theme->call("progress_background_style"));
            progress->add_theme_stylebox_override("fill", theme->call("progress_fill_style"));
        }
        text_box->add_child(progress);
    }

    Button* close = memnew(Button);
    close->set_text("BACK TO MAIN MENU");
    close->set_custom_minimum_size(Vector2(300, 44));
    close->set_h_size_flags(Control::SIZE_SHRINK_CENTER);
    close->connect("pressed", Callable(this, "_close_achievements"));
    root->add_child(close);

    if (theme != nullptr) {
        theme->call("apply_fonts", achievements_overlay);
        theme->call("apply_deep_panel", panel, theme->get("COLOR_GOLD"));
        theme->call("apply_scroll_container", scroll);
        theme->call("apply_secondary_button", close, theme->get("ICON_BACK_PATH"));
    }
    scroll->grab_focus();
}

void MainMenuNative::close_achievements() {
    if (achievements_overlay != nullptr) achievements_overlay->queue_free();
    achievements_overlay = nullptr;
    if (btn_achievements != nullptr) btn_achievements->grab_focus();
}

Ref<Texture2D> MainMenuNative::achievement_icon(int index) const {
    static Ref<Texture2D> atlas = ResourceLoader::get_singleton()->load(ACHIEVEMENT_EMBLEMS_PATH);
    if (atlas.is_null()) return Ref<Texture2D>();
    Ref<AtlasTexture> icon;
    icon.instantiate();
    icon->set_atlas(atlas);
    icon->set_region(Rect2(Math::clamp(index, 0, 11) * 64, 0, 64, 64));
    return icon;
}

void MainMenuNative::start_mode(const String& mode) {
    if (Node* state = singleton(this, "GameState")) {
        state->set_meta(StringName(RUN_MODE_META), mode);
        if (state->has_method("clear_test_run")) {
            state->call("clear_test_run");
        }
    }
    if (Node* music = singleton(this, "MusicManager")) {
        music->call("stop_music");
    }
    if (get_tree() != nullptr) {
        const Error error = get_tree()->change_scene_to_file(GAME_SCENE_PATH);
        if (error != OK) {
            UtilityFunctions::push_error(vformat("Main menu could not start game scene at %s. Error code: %s", GAME_SCENE_PATH, static_cast<int>(error)));
        }
    }
}

Object* MainMenuNative::space_theme() const {
    static Ref<RefCounted> resource;
    if (resource.is_null()) {
        resource = Ref<RefCounted>(Object::cast_to<RefCounted>(ClassDB::instantiate("SpaceThemeNative")));
    }
    return resource.ptr();
}

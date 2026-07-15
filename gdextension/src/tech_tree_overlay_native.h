#pragma once

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/canvas_layer.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/panel_container.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {

class Label;
class StyleBoxFlat;
class VBoxContainer;

class TechTreeBoardNative : public Control {
    GDCLASS(TechTreeBoardNative, Control)

protected:
    static void _bind_methods();

public:
    Array links;
    Dictionary node_centers;
    Dictionary node_states;

    void _draw() override;
};

class TechTreeTooltipButtonNative : public Button {
    GDCLASS(TechTreeTooltipButtonNative, Button)

protected:
    static void _bind_methods();

public:
    Dictionary tooltip_data;

    Object* _make_custom_tooltip(const String& for_text) const override;
    void set_tooltip_data(const Dictionary& data);
    Dictionary get_tooltip_data() const;

private:
    Ref<StyleBoxFlat> tooltip_panel_style(const Color& border) const;
};

class TechTreeOverlayNative : public CanvasLayer {
    GDCLASS(TechTreeOverlayNative, CanvasLayer)

protected:
    static void _bind_methods();

public:
    void _ready() override;
    void _exit_tree() override;
    void _unhandled_input(const Ref<InputEvent>& event) override;

private:
    bool was_paused = false;
    String selected_id = "solar_lens";
    Dictionary upgrade_by_id;
    Dictionary node_cards;

    Control* root = nullptr;
    Button* close_button = nullptr;
    Button* effects_toggle_button = nullptr;
    PanelContainer* title_panel = nullptr;
    PanelContainer* main_panel = nullptr;
    Panel* selected_panel = nullptr;
    TechTreeBoardNative* tree_board = nullptr;
    VBoxContainer* selected_box = nullptr;
    Label* selected_title = nullptr;
    TextureRect* selected_icon = nullptr;
    Label* selected_status = nullptr;
    Label* selected_cost = nullptr;
    Label* selected_requires = nullptr;
    Label* selected_desc = nullptr;
    Button* unlock_button = nullptr;
    Label* title_label = nullptr;
    Label* xp_label = nullptr;

    void build_overlay();
    void build_selected_panel();
    void build_upgrade_nodes();
    void layout_overlay();
    void layout_upgrade_nodes();
    void select_upgrade(const String& id);
    void refresh_tree();
    void try_unlock_selected();
    void toggle_tech_effects();
    void refresh_effects_toggle_button();
    bool tech_effects_enabled() const;
    void update_unlock_button(const Dictionary& upgrade);
    String upgrade_state(const Dictionary& upgrade) const;
    String upgrade_tag(const Dictionary& upgrade) const;
    String requirement_text(const Dictionary& upgrade) const;
    String upgrade_description(const Dictionary& upgrade) const;
    Dictionary upgrade_tooltip_data(const Dictionary& upgrade) const;
    bool requirements_met(const Dictionary& upgrade) const;
    Array requirements(const Dictionary& upgrade) const;
    int upgrade_tier(const Dictionary& upgrade) const;
    void close_overlay();
    int tech_xp() const;
    void update_xp_label();
    void apply_xp_label_font();
    String format_number(int value) const;
    Ref<Texture2D> load_upgrade_icon(const Dictionary& upgrade) const;
    bool is_apex_upgrade(const String& id) const;
    bool apex_choice_unlocked(const String& except_id = "") const;
    Ref<Texture2D> load_texture(const String& path, const String& fallback_path = "") const;
    Color state_color(const String& state) const;
    void style_upgrade_button(Button* button, const String& state) const;
    Ref<StyleBoxFlat> panel_style(const Color& bg, const Color& border, double radius, double margin_x, double margin_y) const;
    Ref<StyleBoxFlat> button_style(const Color& bg, const Color& border, double radius, int border_width) const;
    Object* space_theme() const;
    Node* game_state() const;
};

}
